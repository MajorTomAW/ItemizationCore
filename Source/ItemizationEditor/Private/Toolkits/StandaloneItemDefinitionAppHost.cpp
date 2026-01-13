// Author: Tom Werner (MajorT), 2025 November


#include "StandaloneItemDefinitionAppHost.h"

#include "ItemDefinitionApp.h"
#include "Toolkits/IItemDefinitionAppHost.h"
#include "Toolkits/Tabs/ItemDefinitionAppTabHost.h"

void FStandaloneItemDefinitionAppHost::Init(const TWeakPtr<FItemDefinitionApp>& InApp)
{
	WeakApp = InApp;
	TabHost = MakeShared<UE::ItemizationEditor::FWorkspaceTabHost>();
}

bool FStandaloneItemDefinitionAppHost::CanToolkitSpawnWorkspaceTab() const
{
	return false;
}

UItemDefinitionBase* FStandaloneItemDefinitionAppHost::GetItemDefinition() const
{
	if (const TSharedPtr<FItemDefinitionApp> AppPtr = WeakApp.Pin())
	{
		return AppPtr->GetItemDefinition();
	}

	return nullptr;
}

FSimpleMulticastDelegate& FStandaloneItemDefinitionAppHost::OnItemDefinitionChanged()
{
	return OnItemDefinitionChangedDelegate;
}

TSharedPtr<IDetailsView> FStandaloneItemDefinitionAppHost::GetAssetDetailsView() const
{
	checkf(false, TEXT("YOu havent implemented this yet"))
	return nullptr;
}

TSharedPtr<UE::ItemizationEditor::FWorkspaceTabHost> FStandaloneItemDefinitionAppHost::GetTabHost() const
{
	return TabHost;
}
