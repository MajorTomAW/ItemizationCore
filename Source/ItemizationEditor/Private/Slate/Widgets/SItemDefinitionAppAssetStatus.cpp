// Author: Tom Werner (MajorT), 2025 November


#include "SItemDefinitionAppAssetStatus.h"

#include "AssetManagerEditorModule.h"
#include "DataValidationModule.h"
#include "FileHelpers.h"
#include "SlateOptMacros.h"
#include "Items/ItemDefinitionBase.h"
#include "Misc/DataValidation.h"
#include "Styling/ToolBarStyle.h"
#include "Toolkits/IItemDefinitionApp.h"
#include "Widgets/Images/SLayeredImage.h"

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION

void SItemDefinitionAppAssetStatus::Construct(
	const FArguments& InArgs,
	const TSharedPtr<IItemDefinitionApp>& InApp)
{
	WeakApp = InApp;

	//InToolkit->GetOnAssetSavedDelegate().BindRaw(this, &SItemizationEditorAssetStatus::OnAssetSaved);

	const FToolBarStyle& ToolBarStyle = FAppStyle::GetWidgetStyle<FToolBarStyle>("CalloutToolbar");

	SAssignNew(IconWidget, SLayeredImage)
		.Image(this, &ThisClass::GetIconBrush)
		.ColorAndOpacity(FSlateColor::UseForeground());
	IconWidget->AddLayer(TAttribute<const FSlateBrush*>(this, &ThisClass::GetOverlayIconBrush));

	SAssignNew(TextBlock, STextBlock)
		.Text(this, &ThisClass::GetStatusText);

	ChildSlot
	.Padding(FMargin(8.f, 0.f))
	[
		SNew(SButton)
		.ToolTipText(this, &ThisClass::GetToolTipText)
		.ContentPadding(0.f)
		.OnClicked(this, &ThisClass::HandleOnClicked)
		.ButtonStyle(&ToolBarStyle.ButtonStyle)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(ToolBarStyle.IconPadding)
			.VAlign(VAlign_Center)
			.HAlign(HAlign_Center)
			[
				IconWidget.ToSharedRef()
			]

			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(ToolBarStyle.LabelPadding)
			.VAlign(VAlign_Center)
			[
				TextBlock.ToSharedRef()
			]
		]
	];
}

END_SLATE_FUNCTION_BUILD_OPTIMIZATION

const FSlateBrush* SItemDefinitionAppAssetStatus::GetIconBrush() const
{
	return FAppStyle::Get().GetBrush("Blueprint.CompileStatus.Background");
}

const FSlateBrush* SItemDefinitionAppAssetStatus::GetOverlayIconBrush() const
{
	if (AssetStatus == EDataValidationResult::Invalid)
	{
		return FAppStyle::Get().GetBrush("Blueprint.CompileStatus.Overlay.Error");
	}

	if (bHasAnyWarnings)
	{
		return FAppStyle::Get().GetBrush("Blueprint.CompileStatus.Overlay.Warning");
	}

	TArray<UPackage*> DirtyPackages;
	FEditorFileUtils::GetDirtyPackages(DirtyPackages);
	if (DirtyPackages.Contains(WeakApp.Pin()->GetItemDefinition()->GetOutermost()) && !bManuallyValidated)
	{
		return FAppStyle::Get().GetBrush("Blueprint.CompileStatus.Overlay.Unknown");
	}

	return FAppStyle::Get().GetBrush("Blueprint.CompileStatus.Overlay.Good");
}

FText SItemDefinitionAppAssetStatus::GetStatusText() const
{
	if (AssetStatus == EDataValidationResult::Invalid)
	{
		return FText::FromString(TEXT("Bad"));
	}

	if (bHasAnyWarnings)
	{
		return FText::FromString(TEXT("Warning"));
	}

	TArray<UPackage*> DirtyPackages;
	FEditorFileUtils::GetDirtyPackages(DirtyPackages);

	if (DirtyPackages.Contains(WeakApp.Pin()->GetItemDefinition()->GetOutermost()) && !bManuallyValidated)
	{
		return FText::FromString(TEXT("Unknown"));
	}

	return FText::FromString(TEXT("All Ok"));
}

FText SItemDefinitionAppAssetStatus::GetToolTipText() const
{
	TSharedPtr<IItemDefinitionApp> AppPtr = WeakApp.Pin();
	const UItemDefinitionBase* ItemDefinition = AppPtr.IsValid() ? AppPtr->GetItemDefinition() : nullptr;

	if (ItemDefinition == nullptr)
	{
		return FText::FromString(TEXT("No item definition loaded"));
	}

	if (SizeText.IsEmpty())
	{
		return FText::FromString(FString::Printf(
			TEXT("Item Definition: %s\n\nMemory size hasn't been calculated yet"),
			*ItemDefinition->GetPathName()));
	}

	return FText::FromString(FString::Printf(
		TEXT("Item Definition: %s\n\n%s"),
		*ItemDefinition->GetPathName(),
		*SizeText.ToString()));
}

FReply SItemDefinitionAppAssetStatus::HandleOnClicked()
{
	OnAssetSaved();
	bManuallyValidated = true;

	return FReply::Handled();
}

void SItemDefinitionAppAssetStatus::OnAssetSaved()
{
	const TSharedPtr<IItemDefinitionApp> AppPtr = WeakApp.Pin();
	if (!AppPtr.IsValid())
	{
		AssetStatus = EDataValidationResult::NotValidated;
		return;
	}

	const UItemDefinitionBase* ItemDefinition = AppPtr->GetItemDefinition();
	if (!IsValid(ItemDefinition))
	{
		AssetStatus = EDataValidationResult::NotValidated;
		return;
	}

	IDataValidationModule::Get().ValidateAssets({ItemDefinition}, true, EDataValidationUsecase::Manual );

	FDataValidationContext Context;
	AssetStatus = ItemDefinition->IsDataValid(Context);

	bHasAnyWarnings = Context.GetNumWarnings() > 0;
	bManuallyValidated = false;

	RefreshSize();
}

void SItemDefinitionAppAssetStatus::RefreshSize()
{
	FScopedSlowTask SlowTask(0, FText::FromString(TEXT("Refreshing Size")), true);
	SlowTask.MakeDialog();

	SizeText = FText::GetEmpty();

	const UItemDefinitionBase* ItemDefinition = WeakApp.Pin()->GetItemDefinition();

	auto MakeBestSizeString = [](const SIZE_T SizeInBytes)->FString
	{
		FText SizeText;
		if (SizeInBytes < 1000)
		{
			SizeText = FText::AsMemory(SizeInBytes, SI);
		}
		else
		{
			FNumberFormattingOptions FormatOp;
			FormatOp.MaximumFractionalDigits = 3;
			FormatOp.MinimumFractionalDigits = 0;
			FormatOp.MinimumIntegralDigits = 1;

			SizeText = FText::AsMemory(SizeInBytes, &FormatOp, nullptr, SI);
		}

		return SizeText.ToString();
	};

	int32 ComponentsMemoryUsage = sizeof(FItemComponentDataInstance) * ItemDefinition->GetDataList().Num();
	for (const FItemComponentDataInstance* DataInstance : ItemDefinition->GetDataListRaw())
	{
		ComponentsMemoryUsage += sizeof(DataInstance->Component);
	}

	int64 ItemMemoryUsage = 0;
	IAssetManagerEditorModule& AssetManager = IAssetManagerEditorModule::Get();
	FAssetData Asset = FAssetData(ItemDefinition);
	if (!AssetManager.GetIntegerValueForCustomColumn(Asset, IAssetManagerEditorModule::ResourceSizeName, ItemMemoryUsage))
	{
		ItemMemoryUsage = sizeof(ItemDefinition->GetClass());
		ItemMemoryUsage += (ComponentsMemoryUsage);
	}

	SizeText = FText::FromString(FString::Printf(
		TEXT("Total Memory: %s\nData List Memory: %s"),
		*MakeBestSizeString(ItemMemoryUsage),
		*MakeBestSizeString(ComponentsMemoryUsage)));
}
