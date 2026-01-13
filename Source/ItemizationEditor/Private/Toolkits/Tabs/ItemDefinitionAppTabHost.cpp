// Author: Tom Werner (MajorT), 2025 November


#include "Toolkits/Tabs/ItemDefinitionAppTabHost.h"

#define LOCTEXT_NAMESPACE "ItemDefinitionAppTabHost"

namespace UE::ItemizationEditor
{
	const FLazyName FWorkspaceTabHost::OutlinerTabId = "ItemDefinitionEditor_Outliner";

	FOnSpawnTab FWorkspaceTabHost::CreateSpawnTabDelegate(FName InTabId)
	{
		return ::FOnSpawnTab::CreateSP(this, &ThisClass::HandleSpawnDelegate, InTabId);
	}

	TSharedRef<SDockTab> FWorkspaceTabHost::SpawnTab(FName InTabId)
	{
		const FMinorWorkspaceTabConfig* Config = GetTabConfigs().FindByPredicate(
			[InTabId](const FMinorWorkspaceTabConfig& TabConfig)
		{
			return TabConfig.Identifier == InTabId;
		});

		if (Config == nullptr)
		{
			return SNew(SDockTab).Label(LOCTEXT("TabNotFound", "NOT FOUND"));
		}

		TSharedRef<SDockTab> Result = SNew(SDockTab)
			.Label(Config->Label);

		Result->SetOnTabClosed(SDockTab::FOnTabClosedCallback::CreateSP(this, &ThisClass::HandleTabClosed));

		FSpawnedWorkspaceTab* SpawnedTab = SpawnedTabs.FindByPredicate(
			[InTabId] (const FSpawnedWorkspaceTab& Other)
			{
				return Other.Identifier == InTabId;
			});

		if (SpawnedTab)
		{
			ensureMsgf(false, TEXT("Tab '%s' is already spawned!"), *InTabId.ToString());
			SpawnedTab->SpawnedTab = Result;
		}
		else
		{
			SpawnedTabs.Emplace(InTabId, TWeakPtr<SDockTab>(Result));
		}

		OnTabSpawned.Broadcast(FSpawnedWorkspaceTab(InTabId, Result));
		return Result;
	}

	TConstArrayView<FMinorWorkspaceTabConfig> FWorkspaceTabHost::GetTabConfigs() const
	{
		auto BuildTabConfigs = [] () -> TArray<FMinorWorkspaceTabConfig>
		{
			TArray<FMinorWorkspaceTabConfig> TabConfigs;
			return TabConfigs;
		};

		static TArray<FMinorWorkspaceTabConfig> TabConfigs = BuildTabConfigs();
		return TabConfigs;
	}

	TConstArrayView<FSpawnedWorkspaceTab> FWorkspaceTabHost::GetSpawnedTabs() const
	{
		return SpawnedTabs;
	}

	TSharedRef<SDockTab> FWorkspaceTabHost::HandleSpawnDelegate(const FSpawnTabArgs& Args, FName InTabId)
	{
		return SpawnTab(InTabId);
	}

	void FWorkspaceTabHost::HandleTabClosed(TSharedRef<SDockTab> Tab)
	{
		// Broadcast before removing
		for (const FSpawnedWorkspaceTab& SpawnedTab : SpawnedTabs)
		{
			if (SpawnedTab.SpawnedTab == Tab)
			{
				OnTabClosed.Broadcast(SpawnedTab);
			}
		}

		SpawnedTabs.RemoveAllSwap([Tab](const FSpawnedWorkspaceTab& SpawnedTab)
			{
				return SpawnedTab.SpawnedTab == Tab;
			});
	}
}

#undef LOCTEXT_NAMESPACE
