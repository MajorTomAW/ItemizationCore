// Author: Tom Werner (MajorT), 2025

#include "SItemComponentDataGraphPin.h"

#include "Containers/UnrealString.h"
#include "Delegates/Delegate.h"
#include "EdGraph/EdGraphPin.h"
#include "EdGraph/EdGraphSchema.h"
#include "Editor.h"
#include "InstancedStructDetails.h"
#include "Editor/EditorEngine.h"
#include "Engine/UserDefinedStruct.h"
#include "Internationalization/Internationalization.h"
#include "Layout/Margin.h"
#include "Misc/Attribute.h"
#include "Modules/ModuleManager.h"
#include "SGraphPin.h"
#include "ScopedTransaction.h"
#include "Selection.h"
#include "SInstancedStructPicker.h"
#include "SlotBase.h"
#include "StructViewerFilter.h"
#include "StructViewerModule.h"
#include "Items/Data/ItemComponentData.h"
#include "Items/Data/ItemComponentData_Traits.h"
#include "Styling/AppStyle.h"
#include "Types/SlateStructs.h"
#include "UObject/Class.h"
#include "UObject/NameTypes.h"
#include "Widgets/Input/SMenuAnchor.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/SBoxPanel.h"
 
class SWidget;
class UObject;

#define LOCTEXT_NAMESPACE "SItemComponentDataGraphPin"

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION
void SItemComponentDataGraphPin::Construct(
	const FArguments& InArgs,
	UEdGraphPin* InGraphPinObj)
{
	SGraphPin::Construct(SGraphPin::FArguments(), InGraphPinObj);
}
END_SLATE_FUNCTION_BUILD_OPTIMIZATION

FReply SItemComponentDataGraphPin::OnClickUse()
{
	FEditorDelegates::LoadSelectedAssetsIfNeeded.Broadcast();

	UObject* SelectedObject = GEditor->GetSelectedObjects()->GetTop(UScriptStruct::StaticClass());
	if (IsValid(SelectedObject))
	{
		const FScopedTransaction Transaction(NSLOCTEXT("GraphEditor", "ChangeStructPinValue", "Change Struct Pin Value"));
		GraphPinObj->Modify();

		GraphPinObj->GetSchema()->TrySetDefaultObject(*GraphPinObj, SelectedObject);
	}
	
	return FReply::Handled();
}

bool SItemComponentDataGraphPin::AllowSelfPinWidget() const
{
	return false;
}

TSharedRef<SWidget> SItemComponentDataGraphPin::GenerateAssetPicker()
{
	FStructViewerModule& StructViewer = FModuleManager::LoadModuleChecked<FStructViewerModule>("StructViewer");

	// Fil in the options
	FStructViewerInitializationOptions Options;
	Options.Mode = EStructViewerMode::StructPicker;
	Options.bShowNoneOption = false;
	Options.bAllowViewOptions = false;
	Options.bEnableStructDynamicLoading = true;
	Options.NameTypeToDisplay = EStructViewerNameTypeToDisplay::DisplayName;
	Options.bShowUnloadedStructs = true;
	Options.ExtraPickerCommonStructs.Add(FItemComponentData_Traits::StaticStruct());

	const UScriptStruct* MetaStruct = FItemComponentData::StaticStruct();

	// Using the FInstancedStructFilter
	TSharedRef<FInstancedStructFilter> Filter = MakeShared<FInstancedStructFilter>();
	Options.StructFilter = Filter;
	Filter->BaseStruct = MetaStruct;
	Filter->bAllowBaseStruct = false;

	return SNew(SBox)
		.WidthOverride(280.f)
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.FillHeight(1.f)
			.MaxHeight(500.f)
			[
				SNew(SBorder)
				.Padding(4.f)
				.BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
				[
					StructViewer.CreateStructViewer(Options, FOnStructPicked::CreateSP(this, &ThisClass::HandleOnPickedNewStruct))
				]
			]
		];
}

FText SItemComponentDataGraphPin::GetDefaultComboText() const
{
	return LOCTEXT("DefaultComboText", "Select Struct");
}

FOnClicked SItemComponentDataGraphPin::GetOnUseButtonDelegate()
{
	return FOnClicked::CreateSP(this, &ThisClass::OnClickUse);
}

void SItemComponentDataGraphPin::HandleOnPickedNewStruct(const UScriptStruct* ChosenStruct)
{
	if (GraphPinObj->IsPendingKill())
	{
		return;
	}

	FString NewPath;
	if (ChosenStruct)
	{
		NewPath = ChosenStruct->GetPathName();
	}

	if (GraphPinObj->GetDefaultAsString() != NewPath)
	{
		const FScopedTransaction Transaction( NSLOCTEXT("GraphEditor", "ChangeStructPinValue", "Change Struct Pin Value" ) );
		GraphPinObj->Modify();
 
		AssetPickerAnchor->SetIsOpen(false);
		GraphPinObj->GetSchema()->TrySetDefaultObject(*GraphPinObj, const_cast<UScriptStruct*>(ChosenStruct));
	}
}

#undef LOCTEXT_NAMESPACE