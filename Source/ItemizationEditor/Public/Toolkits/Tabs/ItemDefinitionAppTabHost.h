// Author: Tom Werner (MajorT), 2025 November

#pragma once

namespace UE::ItemizationEditor
{
	struct FMinorWorkspaceTabConfig
	{
		FName Identifier;
		FText Label;
		FText Tooltip;
		FSlateIcon Icon;
		FName UISystemId;
	};

	struct FSpawnedWorkspaceTab
	{
		FName Identifier;
		TWeakPtr<SDockTab> SpawnedTab;
	};

	class ITEMIZATIONEDITOR_API FWorkspaceTabHost : public TSharedFromThis<FWorkspaceTabHost>
	{
		using ThisClass = FWorkspaceTabHost;
	public:
		FWorkspaceTabHost() = default;
		virtual ~FWorkspaceTabHost() = default;

		static const FLazyName OutlinerTabId;

	public:
		FOnSpawnTab CreateSpawnTabDelegate(FName InTabId);
		TSharedRef<SDockTab> SpawnTab(FName InTabId);

		TConstArrayView<FMinorWorkspaceTabConfig> GetTabConfigs() const;
		TConstArrayView<FSpawnedWorkspaceTab> GetSpawnedTabs() const;

		DECLARE_MULTICAST_DELEGATE_OneParam(FOnWorkspaceSpawnTab, FSpawnedWorkspaceTab);
		FOnWorkspaceSpawnTab OnTabSpawned;
		FOnWorkspaceSpawnTab OnTabClosed;

	private:
		TSharedRef<SDockTab> HandleSpawnDelegate(const FSpawnTabArgs& Args, FName InTabId);
		void HandleTabClosed(TSharedRef<SDockTab> Tab);
		TArray<FSpawnedWorkspaceTab> SpawnedTabs;
	};
}
