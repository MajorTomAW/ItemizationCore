// Author: Tom Werner (MajorT), 2025 November


#include "Toolkits/ItemDefinitionViewModel.h"
#include "Items/ItemDefinitionBase.h"

FItemDefinitionViewModel::FItemDefinitionViewModel()
	: ItemDefinition(nullptr)
{
}

FItemDefinitionViewModel::~FItemDefinitionViewModel()
{
}

void FItemDefinitionViewModel::Init(UItemDefinitionBase* InItemDefinition)
{
	ItemDefinition = InItemDefinition;
	GEditor->RegisterForUndo(this);
}

UItemDefinitionBase* FItemDefinitionViewModel::GetItemDefinition() const
{
	return ItemDefinition.Get();
}

void FItemDefinitionViewModel::GetDataList(TArray<TSharedPtr<FItemDataUIInfo>>& OutDataList) const
{
	OutDataList.Reset();
	if (UItemDefinitionBase* ItemPtr = ItemDefinition.Get())
	{
		int32 Index = 0;
		for (FItemComponentDataInstance* Instance : ItemPtr->GetDataListRaw_Mutable())
		{
			if (Instance)
			{
				TSharedPtr<FItemDataUIInfo> Entry = MakeShared<FItemDataUIInfo>();
				Entry->DataInstance = Instance;
				Entry->Index = Index;

				if (Instance->Component.IsValid())
				{
					Entry->LabelText = Instance->Component.GetScriptStruct()->GetDisplayNameText();
				}

				OutDataList.Add(Entry);
			}

			Index++;
		}
	}
}

void FItemDefinitionViewModel::ClearSelection()
{
	if (SelectedItemData.IsEmpty())
	{
		return;
	}

	SelectedItemData.Reset();

	TArray<TSharedPtr<FItemDataUIInfo>> Selection;
	OnSelectionChanged.Broadcast(Selection);
}

void FItemDefinitionViewModel::SetSelection(
	const TArray<TSharedPtr<FItemDataUIInfo>>& InSelection)
{
	if (SelectedItemData.Num() == InSelection.Num() && SelectedItemData == InSelection)
	{
		return;
	}

	SelectedItemData.Reset();

	for (const auto& Selected : InSelection)
	{
		if (Selected.Get())
		{
			SelectedItemData.AddUnique(Selected);
		}
	}

	OnSelectionChanged.Broadcast(InSelection);
}

const TArray<TSharedPtr<FItemDataUIInfo>>& FItemDefinitionViewModel::GetSelection() const
{
	return SelectedItemData;
}

bool FItemDefinitionViewModel::IsSelected(const TSharedPtr<FItemDataUIInfo>& InItemData) const
{
	return SelectedItemData.Contains(InItemData);
}

bool FItemDefinitionViewModel::HasSelection() const
{
	return !SelectedItemData.IsEmpty();
}

void FItemDefinitionViewModel::AddItemData(const UScriptStruct* InStruct)
{
	if (UItemDefinitionBase* ItemPtr = ItemDefinition.Get())
	{
		FScopedTransaction Transaction(FText::FromString(TEXT("Add Item Data")));
		FProperty* Property = FindFProperty<FProperty>(UItemDefinitionBase::StaticClass(), GET_MEMBER_NAME_CHECKED(UItemDefinitionBase, DataList));

		ItemDefinition->PreEditChange(Property);

		FItemComponentDataInstance& DataInstance = ItemPtr->DataList.AddDefaulted_GetRef();
		TInstancedStruct<FItemComponentData> Data(InStruct);
		DataInstance.Component = Data;
		DataInstance.ComponentGuid = FGuid::NewGuid();

		FPropertyChangedEvent ChangedEvent(Property, EPropertyChangeType::ArrayAdd);
		ItemDefinition->PostEditChangeProperty(ChangedEvent);



		TSharedPtr<FItemDataUIInfo> Entry = MakeShared<FItemDataUIInfo>();
		Entry->DataInstance = &DataInstance;
		Entry->Index = ItemPtr->DataList.Num() - 1;
		Entry->LabelText = InStruct->GetDisplayNameText();

		OnItemDataAdded.Broadcast(Entry);
	}
}

void FItemDefinitionViewModel::RemoveSelectedItemData()
{
	if (UItemDefinitionBase* ItemPtr = ItemDefinition.Get())
	{
		FScopedTransaction Transaction(FText::FromString(TEXT("Remove Item Data")));
		FProperty* Property = FindFProperty<FProperty>(UItemDefinitionBase::StaticClass(), GET_MEMBER_NAME_CHECKED(UItemDefinitionBase, DataList));
		ItemDefinition->PreEditChange(Property);

		TArray<FGuid> DatasToRemove;
		for (const auto& Data : SelectedItemData)
		{
			if (Data.IsValid() && Data->DataInstance)
			{
				DatasToRemove.Add(Data->DataInstance->ComponentGuid);
			}
		}

		for (const auto& DataId : DatasToRemove)
		{
			TArray<int32> IndicesToRemove;
			for (int32 i = 0; i < ItemPtr->DataList.Num(); i++)
			{
				FItemComponentDataInstance& DataInstance = ItemPtr->DataList[i];
				if (DataInstance.ComponentGuid == DataId)
				{
					IndicesToRemove.Add(i);
				}
			}

			for (const auto& Indices : IndicesToRemove)
			{
				ItemPtr->DataList.RemoveAt(Indices);
			}
		}

		FPropertyChangedEvent ChangedEvent(Property, EPropertyChangeType::ArrayRemove);
		ItemDefinition->PostEditChangeProperty(ChangedEvent);

		OnItemDataRemoved.Broadcast();
		ClearSelection();
	}
}

void FItemDefinitionViewModel::PostUndo(bool bSuccess)
{
	OnAssetChanged.Broadcast();
}

void FItemDefinitionViewModel::PostRedo(bool bSuccess)
{
	OnAssetChanged.Broadcast();
}
