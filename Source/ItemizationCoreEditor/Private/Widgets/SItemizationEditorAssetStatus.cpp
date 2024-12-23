// Copyright Epic Games, Inc. All Rights Reserved.


#include "SItemizationEditorAssetStatus.h"

#include "AssetManagerEditorModule.h"
#include "DataValidationModule.h"
#include "FileHelpers.h"
#include "ItemDefinition.h"
#include "SlateOptMacros.h"
#include "Misc/DataValidation.h"
#include "Styling/ToolBarStyle.h"
#include "Toolkits/ItemizationEditorApplication.h"

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION

void SItemizationEditorAssetStatus::Construct(const FArguments& InArgs, TSharedPtr<FItemizationEditorApplication> InApp)
{
	WeakApp = InApp;
	InApp->GetOnAssetSavedDelegate().BindRaw(this, &SItemizationEditorAssetStatus::OnAssetSaved);

	const FToolBarStyle& ToolBarStyle = FAppStyle::GetWidgetStyle<FToolBarStyle>("CalloutToolbar");
	
	SAssignNew(IconWidget, SLayeredImage)
		.Image(this, &SItemizationEditorAssetStatus::GetIconBrush)
		.ColorAndOpacity(FSlateColor::UseForeground());
	IconWidget->AddLayer(TAttribute<const FSlateBrush*>(this, &SItemizationEditorAssetStatus::GetOverlayIconBrush));

	SAssignNew(TextBlock, STextBlock)
		.Text(this, &SItemizationEditorAssetStatus::GetStatusText);
	
	ChildSlot
	.Padding(FMargin(8.f, 0.f))
	[
		SNew(SButton)
		.ToolTipText(this, &SItemizationEditorAssetStatus::GetToolTipText)
		.ContentPadding(0.f)
		.OnClicked(this, &SItemizationEditorAssetStatus::OnClicked)
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

const FSlateBrush* SItemizationEditorAssetStatus::GetIconBrush() const
{
	return FAppStyle::Get().GetBrush("Blueprint.CompileStatus.Background");
}

const FSlateBrush* SItemizationEditorAssetStatus::GetOverlayIconBrush() const
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

FText SItemizationEditorAssetStatus::GetStatusText() const
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

FText SItemizationEditorAssetStatus::GetToolTipText() const
{
	TSharedPtr<FItemizationEditorApplication> App = WeakApp.Pin();
	const UItemDefinition* ItemDefinition = App.IsValid() ? App->GetItemDefinition() : nullptr;

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

FReply SItemizationEditorAssetStatus::OnClicked()
{
	OnAssetSaved();
	bManuallyValidated = true;
	
	return FReply::Handled();
}

void SItemizationEditorAssetStatus::OnAssetSaved()
{
	TSharedPtr<FItemizationEditorApplication> App = WeakApp.Pin();
	if (!App.IsValid())
	{
		AssetStatus = EDataValidationResult::Valid;
		return;
	}

	IDataValidationModule::Get().ValidateAssets({App->GetItemDefinition()}, true, EDataValidationUsecase::Manual );
	
	FDataValidationContext Context;
	AssetStatus = App->GetItemDefinition()->IsDataValid(Context);

	bHasAnyWarnings = Context.GetNumWarnings() > 0;
	bManuallyValidated = false;

	RefreshSize();
}

void SItemizationEditorAssetStatus::RefreshSize()
{
	FScopedSlowTask SlowTask(0, FText::FromString(TEXT("Refreshing Size")), true);
	SlowTask.MakeDialog();
	
	SizeText = FText::GetEmpty();

	const UItemDefinition* ItemDefinition = WeakApp.Pin()->GetItemDefinition();

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

	int32 ComponentsMemoryUsage = sizeof(FItemComponentDataProxy) * ItemDefinition->ItemComponents.Num();
	for (FItemComponentDataProxy Proxy : ItemDefinition->ItemComponents)
	{
		ComponentsMemoryUsage += sizeof(Proxy.Component);
	}
	int32 DisplayComponentsMemoryUsage = sizeof(FItemComponentDataProxy) * ItemDefinition->DisplayComponents.Num();
	for (FItemComponentDataProxy Proxy : ItemDefinition->DisplayComponents)
	{
		DisplayComponentsMemoryUsage += sizeof(Proxy.Component);
	}

	int64 ItemMemoryUsage = 0;
	IAssetManagerEditorModule& AssetManager = IAssetManagerEditorModule::Get();
	FAssetData Asset = FAssetData(ItemDefinition);
	if (!AssetManager.GetIntegerValueForCustomColumn(Asset, IAssetManagerEditorModule::ResourceSizeName, ItemMemoryUsage))
	{
		ItemMemoryUsage = sizeof(ItemDefinition->GetClass());
		ItemMemoryUsage += (ComponentsMemoryUsage + DisplayComponentsMemoryUsage);
	}

	SizeText = FText::FromString(FString::Printf(
		TEXT("Total Memory: %s\nComponents Memory: %s\nDisplay Components Usage: %s"),
		*MakeBestSizeString(ItemMemoryUsage),
		*MakeBestSizeString(ComponentsMemoryUsage),
		*MakeBestSizeString(DisplayComponentsMemoryUsage)));
}

END_SLATE_FUNCTION_BUILD_OPTIMIZATION
