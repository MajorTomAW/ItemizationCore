// Author: Tom Werner (MajorT), 2025


#include "ItemDefinitionViewModel.h"

#include "Items/ItemDefinitionBase.h"
#include "Slate/Toolkits/SItemComponentDataRow.h"

FItemDefinitionViewModel::FItemDefinitionViewModel()
{
}

FItemDefinitionViewModel::~FItemDefinitionViewModel()
{
	if (GEditor)
	{
		GEditor->UnregisterForUndo(this);
	}
}

TArray<TSharedPtr<FItemComponentInstanceListEntry>> FItemDefinitionViewModel::GetDataList() const
{
	TArray<TSharedPtr<FItemComponentInstanceListEntry>> Result;
	
	if (UItemDefinitionBase* ItemPtr = ItemDefinition.Get())
	{
		for (const FItemComponentDataInstance* Instance : ItemPtr->GetDataListRaw())
		{
			if (Instance)
			{
				TSharedPtr<FItemComponentInstanceListEntry> Entry = MakeShared<FItemComponentInstanceListEntry>();
				Entry->Instance = Instance;

				if (Instance->Component.IsValid())
				{
					Entry->DisplayName = Instance->Component.GetScriptStruct()->GetDisplayNameText();	
				}

				Result.Add(Entry);
			}
			else
			{
				// Log or handle the case where Instance is null
			}
		}
	}

	return Result;
}

void FItemDefinitionViewModel::Init(UItemDefinitionBase* InItemDefinition)
{
	ItemDefinition = InItemDefinition;

	GEditor->RegisterForUndo(this);
}

void FItemDefinitionViewModel::PostUndo(bool bSuccess)
{
	FEditorUndoClient::PostUndo(bSuccess);
}

void FItemDefinitionViewModel::PostRedo(bool bSuccess)
{
	FEditorUndoClient::PostRedo(bSuccess);
}
