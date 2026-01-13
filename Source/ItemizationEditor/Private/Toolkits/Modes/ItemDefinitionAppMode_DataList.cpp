// Author: Tom Werner (MajorT), 2025 November


#include "ItemDefinitionAppMode_DataList.h"

#include "Items/ItemDefinitionBase.h"
#include "IStructureDetailsView.h"
#include "ItemDefinitionEditingSubsystem.h"
#include "Styles/ItemizationEditorStyle.h"
#include "Toolkits/ItemDefinitionApp.h"
#include "Toolkits/ItemDefinitionViewModel.h"
#include "Toolkits/Tabs/ItemDefinitionAppTabFactory_DataList.h"
#include "Toolkits/Tabs/ItemDefinitionAppTabFactory_DataListDetails.h"

class FItemDefinitionApp;

#define LOCTEXT_NAMESPACE "ItemDefinitionAppMode_DataList"

const FName FItemDefinitionAppMode_DataList::ModeId = "ItemDefinitionAppMode_DataList";

FItemDefinitionAppMode_DataList::FItemDefinitionAppMode_DataList(
	const TSharedPtr<IItemDefinitionApp>& InApp)
		: FItemDefinitionAppMode(UE::ItemizationEditor::FItemDefinitionAppModeInfo(
			ModeId,
			LOCTEXT("ModeLabel", "Data List"),
			LOCTEXT("ModeTooltip", "The mode to view and edit the item's data list."),
			FSlateIcon(FItemizationEditorStyle::Get().GetStyleSetName(), "Icons.DataList"),
			1),
			InApp)
{
	using namespace UE::ItemizationEditor;

	TabLayout = FTabManager::NewLayout("ItemDefinitionAppMode_DataList_Layout_v1.1.1")
	->AddArea
	(
		FTabManager::NewPrimaryArea()
		->SetOrientation(Orient_Vertical)
		->Split
		(
			FTabManager::NewSplitter()
			->SetOrientation(Orient_Horizontal)

			// Left Side: Data List
			->Split
			(
				FTabManager::NewStack()
				->SetSizeCoefficient(0.4f)
				->AddTab(Ids::TabId_DataList, ETabState::OpenedTab)
				->SetHideTabWell(true)
			)

			// Right Side: Selection Details
			->Split
			(
				FTabManager::NewStack()
				->SetSizeCoefficient(0.6f)
				->AddTab(Ids::TabId_DataListDetails, ETabState::OpenedTab)
			)
		)
	);

	TabSet.RegisterFactory(MakeShared<FItemDefinitionAppTabFactory_DataList>(InApp));
	TabSet.RegisterFactory(MakeShared<FItemDefinitionAppTabFactory_DataListDetails>(InApp));

	AddModesToolbar(ToolbarExtender);
}

void FItemDefinitionAppMode_DataList::PostActivateMode()
{
	const TSharedPtr<FItemDefinitionApp> AppPtr = GetApp<FItemDefinitionApp>();
	UItemDefinitionBase* ItemDefinition = AppPtr->GetItemDefinition();
	CachedItemDefinition = ItemDefinition;

	if (ItemDefinition)
	{
		if (UItemDefinitionEditingSubsystem* ItemEdSub = UItemDefinitionEditingSubsystem::Get())
		{
			TSharedRef<FItemDefinitionViewModel> VM = ItemEdSub->FindOrAddViewModel(ItemDefinition);
			VM->GetOnSelectionChanged().AddRaw(this, &ThisClass::HandleModelSelectionChanged);
		}
	}
}

void FItemDefinitionAppMode_DataList::PreDeactivateMode()
{
	if (CachedItemDefinition.IsValid())
	{
		if (UItemDefinitionEditingSubsystem* ItemEdSub = UItemDefinitionEditingSubsystem::Get())
		{
			TSharedRef<FItemDefinitionViewModel> VM = ItemEdSub->FindOrAddViewModel(CachedItemDefinition.Get());

			VM->GetOnAssetChanged().RemoveAll(this);
			VM->GetOnItemDataChanged().RemoveAll(this);
			VM->GetOnItemDataAdded().RemoveAll(this);
			VM->GetOnSelectionChanged().RemoveAll(this);
		}
	}
}

void FItemDefinitionAppMode_DataList::HandleModelSelectionChanged(const TArray<TSharedPtr<FItemDataUIInfo>>& Selection)
{
	if (TSharedPtr<IStructureDetailsView> DetailsView = GetDataListSelectionView())
	{
		if (Selection.IsEmpty())
		{
			DetailsView->SetStructureData(nullptr);
			return;
		}

		TSharedPtr<FItemDataUIInfo> First = Selection[0];

		if (!First.IsValid() || !First->DataInstance || !First->DataInstance->Component.IsValid())
		{
			return;
		}

		TSharedPtr<FStructOnScope> StructScope = MakeShared<FStructOnScope>(
			First->DataInstance->Component.GetScriptStruct(),
			First->DataInstance->Component.GetMutableMemory());

		DetailsView->SetStructureData(StructScope);
	}
}

TSharedPtr<IStructureDetailsView> FItemDefinitionAppMode_DataList::GetDataListSelectionView()
{
	if (TSharedPtr<FItemDefinitionApp> AppPtr = GetApp<FItemDefinitionApp>())
	{
		return AppPtr->GetDataListDetailsView();
	}

	return nullptr;
}

#undef LOCTEXT_NAMESPACE
