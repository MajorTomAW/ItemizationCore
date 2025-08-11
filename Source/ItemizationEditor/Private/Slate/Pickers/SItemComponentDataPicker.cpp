// Author: Tom Werner (MajorT), 2025


#include "SItemComponentDataPicker.h"

#include "DetailLayoutBuilder.h"
#include "DetailWidgetRow.h"
#include "IDetailChildrenBuilder.h"
#include "InstancedStruct.h"
#include "InstancedStructDetails.h"
#include "IPropertyUtilities.h"
#include "PropertyCustomizationHelpers.h"
#include "StructViewerFilter.h"
#include "StructViewerModule.h"
#include "Customization/ItemComponentDataStructFilter.h"
#include "Items/ItemDefinitionBase.h"
#include "StructUtils/InstancedStruct.h"
#include "Styles/ItemizationEditorStyle.h"
#include "Styling/SlateIconFinder.h"

#define LOCTEXT_NAMESPACE "ItemizationEditor"


BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION

void SItemComponentDataPicker::Construct(
	const FArguments& InArgs,
	TSharedPtr<IPropertyHandle> InStructProperty,
	TSharedPtr<IPropertyHandle> InStructInstanceProperty,
	TSharedPtr<IPropertyUtilities> InPropertyUtils)
{
	static const FName NAME_BaseStruct = "BaseStruct";
	static const FName NAME_StructTypeConst = "StructTypeConst";

	OnStructPicked = InArgs._OnStructPicked;
	StructProperty = MoveTemp(InStructProperty);
	InstanceProperty = MoveTemp(InStructInstanceProperty);
	PropUtils = MoveTemp(InPropertyUtils);

	if (!StructProperty.IsValid() || !PropUtils.IsValid())
	{
		return;
	}

	const bool bEnableStructSelection = StructProperty->IsEditable() && !StructProperty->HasMetaData(NAME_StructTypeConst);

	BaseScriptStruct = nullptr;
	{
		const FString& BaseStructName = StructProperty->GetMetaData(NAME_BaseStruct);
		if (!BaseStructName.IsEmpty())
		{
			if (UScriptStruct* Struct = UClass::TryFindTypeSlow<UScriptStruct>(BaseStructName))
			{
				BaseScriptStruct = Struct;
			}
			else
			{
				BaseScriptStruct = LoadObject<UScriptStruct>(nullptr, *BaseStructName);
			}
		}
	}

	this->ChildSlot
	[
		SAssignNew(ComboButton, SComboButton)
		.OnGetMenuContent(this, &ThisClass::HandleGenerateStructPicker)
		.ContentPadding(0.f)
		.IsEnabled(bEnableStructSelection)
		.ButtonContent()
		[
			SNew(SHorizontalBox)

			// Icon
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.VAlign(VAlign_Center)
			.Padding(0.f, 0.f, 4.f, 0.f)
			[
				SNew(SImage)
				.Image(this, &ThisClass::GetDisplayValueIcon)
			]

			// Label
			+ SHorizontalBox::Slot()
			.VAlign(VAlign_Center)
			[
				SNew(STextBlock)
				.Font(IDetailLayoutBuilder::GetDetailFont())
				.Text(this, &ThisClass::GetDisplayValueText)
				.ToolTipText(this, &ThisClass::GetDisplayValueToolTipText)
			]
		]
	];
}
END_SLATE_FUNCTION_BUILD_OPTIMIZATION

TSharedRef<SWidget> SItemComponentDataPicker::HandleGenerateStructPicker()
{
	static const FName NAME_ExcludeBaseStruct = "ExcludeBaseStruct";
	static const FName NAME_HideViewOptions = "HideViewOptions";
	static const FName NAME_ShowTreeView = "ShowTreeView";
	
	const bool bExcludeBaseStruct = InstanceProperty->HasMetaData(NAME_ExcludeBaseStruct);
	const bool bAllowNone = !(InstanceProperty->GetMetaDataProperty()->PropertyFlags & CPF_NoClear);
	const bool bHideViewOptions = InstanceProperty->HasMetaData(NAME_HideViewOptions);
	const bool bShowTreeView = InstanceProperty->HasMetaData(NAME_ShowTreeView);

	TSharedRef<FItemComponentDataStructFilter> Filter = MakeShared<FItemComponentDataStructFilter>();
	Filter->BaseStruct = BaseScriptStruct;
	Filter->bAllowBaseStruct = !bExcludeBaseStruct;

	// Only allow user defined structs when BaseStruct is not set.
	Filter->bAllowUserDefinedStructs = BaseScriptStruct.IsExplicitlyNull();


	// Find per object disallowed structs
	const UItemDefinitionBase* ItemDefinition = nullptr;
	TArray<UObject*> Outers;
	InstanceProperty->GetOuterObjects(Outers);
	for (UObject* OuterObj : Outers)
	{
		if (!IsValid(OuterObj) || !OuterObj->IsA<UItemDefinitionBase>())
		{
			continue;
		}

		ItemDefinition = Cast<UItemDefinitionBase>(OuterObj);
		break;
	}

	if (IsValid(ItemDefinition))
	{
		TArray<TSoftObjectPtr<const UScriptStruct>> DisallowedDataTypes = ItemDefinition->GetDisallowedDataTypes();
		if (!DisallowedDataTypes.IsEmpty())
		{
			Filter->DisallowedStructs = DisallowedDataTypes;	
		}
	}

	// Construct our filter
	if (GEditor && InstanceProperty)
	{
		FAssetReferenceFilterContext AssetReferenceFilterContext;

		TArray<UPackage*> OuterPackages;
		InstanceProperty->GetOuterPackages(OuterPackages);
		for (UPackage* OuterPackage : OuterPackages)
		{
			AssetReferenceFilterContext.AddReferencingAsset(FAssetData(OuterPackage));
		}

		Filter->AssetReferenceFilter = GEditor->MakeAssetReferenceFilter(AssetReferenceFilterContext);

		auto SoftPointerTransform = [](const UScriptStruct* InStruct) -> TSoftObjectPtr<const UScriptStruct>
		{
			return InStruct;
		};
		
		Algo::Transform(PropertyCustomizationHelpers::GetStructsFromMetadataString(InstanceProperty->GetMetaData("AllowedClasses")), Filter->AllowedStructs, SoftPointerTransform);
		Algo::Transform(PropertyCustomizationHelpers::GetStructsFromMetadataString(InstanceProperty->GetMetaData("DisallowedClasses")), Filter->DisallowedStructs, SoftPointerTransform);

		TArray<UObject*> OwningObjects;
		InstanceProperty->GetOuterObjects(OwningObjects);
		for (UObject* OwningObject : OwningObjects)
		{
			if (OwningObject != nullptr)
			{
				const FString GetAllowedClassesFunctionName = InstanceProperty->GetMetaData("GetAllowedClasses");
				if (!GetAllowedClassesFunctionName.IsEmpty())
				{
					const UFunction* GetAllowedClassesFunction = OwningObject ? OwningObject->FindFunction(*GetAllowedClassesFunctionName) : nullptr;
					if (GetAllowedClassesFunction != nullptr)
					{
						DECLARE_DELEGATE_RetVal(TArray<TSoftObjectPtr<UScriptStruct>>, FGetAllowedClasses);
						Filter->AllowedStructs.Append(FGetAllowedClasses::CreateUFunction(OwningObject, GetAllowedClassesFunction->GetFName()).Execute());
					}
				}
		
				const FString GetDisallowedClassesFunctionName = InstanceProperty->GetMetaData("GetDisallowedClasses");
				if (!GetDisallowedClassesFunctionName.IsEmpty())
				{
					const UFunction* GetDisallowedClassesFunction = OwningObject ? OwningObject->FindFunction(*GetDisallowedClassesFunctionName) : nullptr;
					if (GetDisallowedClassesFunction != nullptr)
					{
						DECLARE_DELEGATE_RetVal(TArray<TSoftObjectPtr<UScriptStruct>>, FGetDisallowedClasses);
						Filter->DisallowedStructs.Append(FGetDisallowedClasses::CreateUFunction(OwningObject, GetDisallowedClassesFunction->GetFName()).Execute());
					}
				}
			}
		}
	}

	const UScriptStruct* SelectedStruct = nullptr;
	GetCommonScriptStruct(StructProperty, SelectedStruct);

	FStructViewerInitializationOptions Options;
	Options.bShowNoneOption = bAllowNone;
	Options.StructFilter = Filter;
	Options.NameTypeToDisplay = EStructViewerNameTypeToDisplay::DisplayName;
	Options.DisplayMode = bShowTreeView ? EStructViewerDisplayMode::TreeView : EStructViewerDisplayMode::ListView;
	Options.bAllowViewOptions = !bHideViewOptions;
	Options.SelectedStruct = SelectedStruct;
	Options.PropertyHandle = StructProperty;

	const FOnStructPicked OnPicked(FOnStructPicked::CreateSP(this, &ThisClass::HandleStructPicked));

	return SNew(SBox)
		.WidthOverride(280)
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.AutoHeight()
			.MaxHeight(500)
			[
				FModuleManager::LoadModuleChecked<FStructViewerModule>("StructViewer").CreateStructViewer(Options, OnPicked)
			]
		];
}

void SItemComponentDataPicker::HandleStructPicked(const UScriptStruct* InStruct)
{
	if (StructProperty && StructProperty->IsValidHandle())
	{
		FScopedTransaction Transaction(LOCTEXT("OnStructPicked", "Set Struct"));

		StructProperty->NotifyPreChange();

		StructProperty->EnumerateRawData([InStruct](void* RawData, const int32 /*DataIndex*/, const int32 /*NumDatas*/)
		{
			if (FInstancedStruct* InstancedStruct = static_cast<FInstancedStruct*>(RawData))
			{
				InstancedStruct->InitializeAs(InStruct);
			}
			return true;
		});

		StructProperty->NotifyPostChange(EPropertyChangeType::ValueSet);
		StructProperty->NotifyFinishedChangingProperties();

		// After the type has changed, let's expand, so that the user can edit the newly appeared child properties
		StructProperty->SetExpanded(true);

		// The Property tree will be invalid after changing the struct type, force update.
		if (PropUtils.IsValid())
		{
			PropUtils->ForceRefresh();
		}
	}

	ComboButton->SetIsOpen(false);
	OnStructPicked.ExecuteIfBound(InStruct);
}

const FSlateBrush* SItemComponentDataPicker::GetDisplayValueIcon() const
{
	const UScriptStruct* CommonStruct = nullptr;
	if (GetCommonScriptStruct(StructProperty, CommonStruct) == FPropertyAccess::Success)
	{
		if (CommonStruct)
		{
			const FSlateBrush* Brush = nullptr;
			
			// Try to get the FItemComponentData struct from the common struct
			if (UStruct* OwnerStruct = CommonStruct->GetOwnerStruct())
			{
				Brush = FItemizationEditorStyle::Get()->GetBrush(OwnerStruct->GetFName());
			}

			if (
				/*Brush Invalid*/ Brush == nullptr ||
				/*Brush not set*/ !Brush->IsSet() ||
				/*Brush is default brush*/ Brush->GetResourceName().ToString().EndsWith(TEXT("Checkerboard.png")))
			{
				Brush = FSlateIconFinder::FindIconBrushForClass(CommonStruct->GetClass());
			}
			
			return Brush;
		}
		
		return FSlateIconFinder::FindIconBrushForClass(UScriptStruct::StaticClass());
	}

	return nullptr;
}

FText SItemComponentDataPicker::GetDisplayValueText() const
{
	const UScriptStruct* CommonStruct = nullptr;
	const FPropertyAccess::Result Result = GetCommonScriptStruct(StructProperty, CommonStruct);

	if (Result == FPropertyAccess::Success)
	{
		if (CommonStruct)
		{
			return CommonStruct->GetDisplayNameText();
		}
		return LOCTEXT("NullScriptStruct", "None");
	}
	if (Result == FPropertyAccess::MultipleValues)
	{
		return LOCTEXT("MultipleValues", "Multiple Values");
	}

	return FText::GetEmpty();
}

FText SItemComponentDataPicker::GetDisplayValueToolTipText() const
{
	const UScriptStruct* CommonStruct = nullptr;
	const FPropertyAccess::Result Result = GetCommonScriptStruct(StructProperty, CommonStruct);
	if (CommonStruct && Result == FPropertyAccess::Success)
	{
		return CommonStruct->GetToolTipText();
	}

	return GetDisplayValueText();
}




#undef LOCTEXT_NAMESPACE