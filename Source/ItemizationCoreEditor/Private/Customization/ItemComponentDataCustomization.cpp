// Fill out your copyright notice in the Description page of Project Settings.


#include "ItemComponentDataCustomization.h"

#include "DetailLayoutBuilder.h"
#include "DetailWidgetRow.h"
#include "IDetailChildrenBuilder.h"
#include "InstancedStruct.h"
#include "InstancedStructDetails.h"
#include "ItemizationCoreEditor.h"
#include "ItemizationCoreEditorHelpers.h"
#include "Components/ItemComponentData.h"
#include "StructViewerFilter.h"
#include "StructViewerModule.h"
#include "StructUtils/InstancedStruct.h"
#include "Styles/ItemizationEditorStyle.h"
#include "Styling/SlateIconFinder.h"

static const FName NAME_ExcludeBaseStruct = "ExcludeBaseStruct";
static const FName NAME_HideViewOptions = "HideViewOptions";
static const FName NAME_ShowTreeView = "ShowTreeView";
static const FName NAME_DisallowedStructs = "DisallowedStructs";
static const FName NAME_SingletonComponent = "SingletonComponent";
static const FName NAME_CosmeticComponent = "CosmeticComponent";
static const FName NAME_AllowCosmeticComponents = "AllowCosmeticComponents";
static const FName NAME_AllowNonCosmeticComponents = "AllowNonCosmeticComponents";

#define LOCTEXT_NAMESPACE "ItemizationEditor"

auto GetCommonScriptStruct = [](TSharedPtr<IPropertyHandle> StructProperty, const UScriptStruct*& OutCommonStruct)
{
	bool bHasResult = false;
	bool bHasMultipleValues = false;
	
	StructProperty->EnumerateConstRawData([&OutCommonStruct, &bHasResult, &bHasMultipleValues](const void* RawData, const int32 /*DataIndex*/, const int32 /*NumDatas*/)
	{
		if (const FInstancedStruct* InstancedStruct = static_cast<const FInstancedStruct*>(RawData))
		{
			const UScriptStruct* Struct = InstancedStruct->GetScriptStruct();

			if (!bHasResult)
			{
				OutCommonStruct = Struct;
			}
			else if (OutCommonStruct != Struct)
			{
				bHasMultipleValues = true;
			}

			bHasResult = true;
		}

		return true;
	});

	if (bHasMultipleValues)
	{
		return FPropertyAccess::MultipleValues;
	}
	
	return bHasResult ? FPropertyAccess::Success : FPropertyAccess::Fail;
};

class FItemComponentDataStructFilter : public IStructViewerFilter
{
public:
	/** optimal outer class for the filter */
	const UClass* OuterClass = nullptr;
	
	/** The base struct for the property that classes must be a child-of. */
	const UScriptStruct* BaseStruct = nullptr;

	/** Flag controlling whether we allow to select the BaseStruct. */
	bool bAllowBaseStruct = true;

	/** Flag controlling whether we allow cosmetic structs. */
	bool bAllowCosmeticStructs = false;

	/** Flag controlling whether we allow non cosmetic structs. */
	bool bAllowNonCosmeticStructs = false;

	/** Disallowed structs. */
	TArray<const UScriptStruct*> DisallowedStructs;

	/** Disallowed structs (wont call IsChildOf(). */
	TArray<const UScriptStruct*> DisallowedStructsExact;

public:
	virtual bool IsStructAllowed(const FStructViewerInitializationOptions& InInitOptions, const UScriptStruct* InStruct, TSharedRef<class FStructViewerFilterFuncs> InFilterFuncs) override
	{
		if (!InStruct->IsChildOf(BaseStruct))
		{
			return false;
		}
		
		for (const UScriptStruct* DisallowedStruct : DisallowedStructs)
		{
			if (InStruct->IsChildOf(DisallowedStruct))
			{
				return false;
			}
		}

		for (const UScriptStruct* DisallowedStruct : DisallowedStructsExact)
		{
			if (InStruct == DisallowedStruct)
			{
				return false;
			}
		}

		const bool bIsCosmetic = InStruct->HasMetaData(NAME_CosmeticComponent) ? InStruct->GetBoolMetaData(NAME_CosmeticComponent) : false;
		if (bIsCosmetic && !bAllowCosmeticStructs)
		{
			return false;
		}

		if (!bIsCosmetic && !bAllowNonCosmeticStructs)
		{
			return false;
		}

		if (OuterClass)
		{
			TSharedPtr<UE::ItemizationCore::Editor::FItemizationEditorAssetConfig> Config = IItemizationCoreEditorModule::Get().GetAssetConfig(OuterClass);

			if (Config.IsValid())
			{
				if (!Config->CanShowItemComponent(InStruct->GetFName()))
				{
					return false;
				}
			}
		}
		
		if (InStruct == BaseStruct)
		{
			return bAllowBaseStruct;
		}

		if (InStruct->HasMetaData(TEXT("Hidden")))
		{
			return false;
		}

		// Query the native struct to see if it has the correct parent type (if any)
		return !BaseStruct || InStruct->IsChildOf(BaseStruct);
	}

	virtual bool IsUnloadedStructAllowed(const FStructViewerInitializationOptions& InInitOptions, const FSoftObjectPath& InStructPath, TSharedRef<class FStructViewerFilterFuncs> InFilterFuncs) override
	{
		return false;
	}
};


TSharedRef<IPropertyTypeCustomization> FItemComponentDataCustomization::MakeInstance()
{
	return MakeShareable(new FItemComponentDataCustomization);
}

void FItemComponentDataCustomization::CustomizeHeader(
	TSharedRef<IPropertyHandle> PropertyHandle, FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& CustomizationUtils)
{
	static const FName NAME_StructTypeConst = "StructTypeConst";
	
	ProxyProperty = PropertyHandle;
	StructProperty = PropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FItemComponentDataProxy, Component));
	check(StructProperty);

	ArrayProperty = ProxyProperty->GetParentHandle()->AsArray();

	const bool bEnableStructSelection = !ProxyProperty->HasMetaData(NAME_StructTypeConst);

	SAssignNew(HitBox, SBorder)
	.BorderImage(FAppStyle::Get().GetBrush("NoBorder"))
	.Padding(0.f)
	.Content()
	[
		SNew(SHorizontalBox)
		.ToolTipText(this, &FThis::GetTooltipText)
		
		// Icon
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.Padding(FMargin(0.f, 0.f, 4.f, 0.f))
		.VAlign(VAlign_Center)
		[
			SNew(SImage)
			.DesiredSizeOverride(FVector2d(16.f))
			.Image(FItemizationEditorStyle::Get()->GetBrush("Icons.Components"))
		]

		// Label
		+ SHorizontalBox::Slot()
		.Padding(0.f)
		.AutoWidth()
		.VAlign(VAlign_Center)
		[
			SNew(STextBlock)
			.Text(this, &FThis::GetDisplayValueString)
		]

		// Index: [0]
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.Padding(FMargin(12.f, 0.f, 0.f, 0.f))
		.VAlign(VAlign_Center)
		[
			SNew(STextBlock)
			.Font(IDetailLayoutBuilder::GetDetailFont())
			.Text(this, &FThis::GetDisplayIndexString)
			.Visibility(this, &FThis::GetHiddenDataVisibility)
		]
	];
	
	HeaderRow
	.NameContent()
	.VAlign(VAlign_Center)
	.HAlign(HAlign_Fill)
	[
		HitBox.ToSharedRef()
	]
	.ValueContent()
	.MinDesiredWidth(250.f)
	.VAlign(VAlign_Center)
	[
		SAssignNew(ComboButton, SComboButton)
		.OnGetMenuContent(this, &FThis::GenerateStructPicker)
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
				.Image(this, &FThis::GetDisplayValueIcon)
			]

			// Label
			+ SHorizontalBox::Slot()
			.VAlign(VAlign_Center)
			[
				SNew(STextBlock)
				.Font(IDetailLayoutBuilder::GetDetailFont())
				.Text(this, &FThis::GetDisplayValueString)
				.ToolTipText(this, &FThis::GetTooltipText)
			]
		]
	];
}

void FItemComponentDataCustomization::CustomizeChildren(
	TSharedRef<IPropertyHandle> PropertyHandle, IDetailChildrenBuilder& ChildBuilder, IPropertyTypeCustomizationUtils& CustomizationUtils)
{
	TSharedRef<FInstancedStructDataDetails> DataDetails = MakeShared<FInstancedStructDataDetails>(StructProperty);
	ChildBuilder.AddCustomBuilder(DataDetails);
}

TSharedRef<SWidget> FItemComponentDataCustomization::GenerateStructPicker()
{
	const bool bExcludeBaseStruct = /*ProxyProperty->HasMetaData(NAME_ExcludeBaseStruct);*/ true;
	const bool bAllowNone = !(ProxyProperty->GetMetaDataProperty()->PropertyFlags & CPF_NoClear);
	const bool bHideViewOptions = ProxyProperty->HasMetaData(NAME_HideViewOptions);
	const bool bShowTreeView = ProxyProperty->HasMetaData(NAME_ShowTreeView);
	const bool bAllowCosmeticComponents = ProxyProperty->HasMetaData(NAME_AllowCosmeticComponents) ?  ProxyProperty->GetBoolMetaData(NAME_AllowCosmeticComponents) : true;
	const bool bAllowNonCosmeticComponents = ProxyProperty->HasMetaData(NAME_AllowNonCosmeticComponents) ?  ProxyProperty->GetBoolMetaData(NAME_AllowNonCosmeticComponents) : true;

	TSharedRef<FItemComponentDataStructFilter> StructFilter = MakeShared<FItemComponentDataStructFilter>();
	StructFilter->BaseStruct = TBaseStructure<FItemComponentData>::Get();
	StructFilter->bAllowBaseStruct = !bExcludeBaseStruct;
	StructFilter->bAllowCosmeticStructs = bAllowCosmeticComponents;
	StructFilter->bAllowNonCosmeticStructs = bAllowNonCosmeticComponents;
	StructFilter->OuterClass = ProxyProperty->GetOuterBaseClass();

	// Find disallowed structs
	for (TSharedPtr<IPropertyHandle> Handle = StructProperty; Handle.IsValid(); Handle = Handle->GetParentHandle())
	{
		const FString& DisallowedStructs = Handle->GetMetaData(NAME_DisallowedStructs);
		if (!DisallowedStructs.IsEmpty())
		{
			TArray<FString> DisallowedStructNames;
			DisallowedStructs.ParseIntoArray(DisallowedStructNames, TEXT(","));

			for (const FString& DisallowedStructName : DisallowedStructNames)
			{
				UScriptStruct* ScriptStruct = FindObject<UScriptStruct>(nullptr, *DisallowedStructName, /*ExactClass*/false);
				if (ScriptStruct == nullptr)
				{
					ScriptStruct = LoadObject<UScriptStruct>(nullptr, *DisallowedStructName);
				}
				if (ScriptStruct)
				{
					StructFilter->DisallowedStructs.Add(ScriptStruct);
				}
			}

			break;
		}
	}

	// Find existing singleton structs and add them to the disallowed list
	if (ArrayProperty && ArrayProperty.Get())
	{
		uint32 NumElements;
		if (ArrayProperty->GetNumElements(NumElements) == FPropertyAccess::Success)
		{
			UE_LOG(LogTemp, Warning, TEXT("NumElements: %d"), NumElements);
			
			for (uint32 Index = 0; Index < NumElements; ++Index)
			{
				TSharedPtr<IPropertyHandle> ElementProxyHandle = ArrayProperty->GetElement(Index);
				if (!ElementProxyHandle->IsValidHandle())
				{
					continue;
				}

				TSharedPtr<IPropertyHandle> ElementStructHandle = ElementProxyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FItemComponentDataProxy, Component));

				const UScriptStruct* CommonStruct = nullptr;
				const FPropertyAccess::Result Result = GetCommonScriptStruct(ElementStructHandle, CommonStruct);
				if (Result != FPropertyAccess::Success || CommonStruct == nullptr)
				{
					continue;
				}

				// Now check if the CommonStruct is a singleton component, if so, add it to the disallowed list
				if (CommonStruct->HasMetaData(NAME_SingletonComponent) && CommonStruct->GetBoolMetaData(NAME_SingletonComponent))
				{
					StructFilter->DisallowedStructsExact.Add(CommonStruct);
				}
			}
		}
	}
	
	
	FStructViewerInitializationOptions Options;
	Options.Mode = EStructViewerMode::StructPicker;
	Options.bShowNoneOption = bAllowNone;
	Options.bAllowViewOptions = !bHideViewOptions;
	Options.DisplayMode = bShowTreeView ? EStructViewerDisplayMode::TreeView : EStructViewerDisplayMode::ListView;
	Options.NameTypeToDisplay = EStructViewerNameTypeToDisplay::DisplayName;
	Options.StructFilter = StructFilter;

	FOnStructPicked OnStructPicked(FOnStructPicked::CreateSP(this, &FThis::OnStructPicked));
	
	return SNew(SBox)
		.WidthOverride(280.f)
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.AutoHeight()
			.MaxHeight(480.f)
			[
				FModuleManager::LoadModuleChecked<FStructViewerModule>("StructViewer").CreateStructViewer(Options, OnStructPicked)
			]
		];
}

void FItemComponentDataCustomization::OnStructPicked(const UScriptStruct* InStruct)
{
	if (StructProperty && StructProperty->IsValidHandle())
	{
		FScopedTransaction Transaction(LOCTEXT("OnStructPicked", "Set Struct"));

		StructProperty->NotifyPreChange();
		StructProperty->EnumerateRawData([InStruct](void* RawData, const int32 /*DataIndex*/, const int32 /*NumData*/)
		{
			if (FInstancedStruct* InstancedStruct = static_cast<FInstancedStruct*>(RawData))
			{
				InstancedStruct->InitializeAs(InStruct);
			}
			return true;
		});

		StructProperty->NotifyPostChange(EPropertyChangeType::ValueSet);
		StructProperty->NotifyFinishedChangingProperties();
	}

	ComboButton->SetIsOpen(false);
}

const FSlateBrush* FItemComponentDataCustomization::GetDisplayValueIcon() const
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

EVisibility FItemComponentDataCustomization::GetHiddenDataVisibility() const
{
	if (!ArrayProperty.IsValid())
	{
		return EVisibility::Collapsed;
	}
	
	return (ComboButton->IsHovered() || HitBox->IsHovered()) ? EVisibility::Visible : EVisibility::Collapsed;
}

FText FItemComponentDataCustomization::GetDisplayValueString() const
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

FText FItemComponentDataCustomization::GetDisplayValueStringWrapped() const
{
	const FText DisplayValueString = GetDisplayValueString();

	if (ArrayProperty && ArrayProperty.IsValid())
	{
		const int32 Index = ProxyProperty->GetIndexInArray();
		return FText::Format(LOCTEXT("ArrayElementFormat_Array", "Index [ {0} ] :	{1}"), FText::AsNumber(Index), DisplayValueString);
	}
	
	return DisplayValueString;
}

FText FItemComponentDataCustomization::GetDisplayIndexString() const
{
	if (ArrayProperty && ArrayProperty.IsValid())
	{
		const int32 Index = ProxyProperty->GetIndexInArray();
		return FText::Format(LOCTEXT("ArrayElementFormat", "Index [ {0} ]"), FText::AsNumber(Index));
	}

	return FText::GetEmpty();
}

FText FItemComponentDataCustomization::GetTooltipText() const
{
	const UScriptStruct* CommonStruct = nullptr;
	const FPropertyAccess::Result Result = GetCommonScriptStruct(StructProperty, CommonStruct);
	if (CommonStruct && Result == FPropertyAccess::Success)
	{
		return CommonStruct->GetToolTipText();
	}

	return GetDisplayValueString();
}

#undef LOCTEXT_NAMESPACE
