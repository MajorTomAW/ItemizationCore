// Author: Tom Werner (MajorT), 2025


#include "StandaloneItemDefinitionEditorHost.h"

#include "ItemDefinitionApplication.h"
#include "Toolkits/ItemDefinitionEditorWorkspaceTabHost.h"

void FStandaloneItemDefinitionEditorHost::Init(const TWeakPtr<FItemDefinitionApplication>& InWeakEditor)
{
	WeakEditor = InWeakEditor;
	TabHost = MakeShared<UE::ItemizationEditor::FWorkspaceTabHost>();
}

bool FStandaloneItemDefinitionEditorHost::CanToolkitSpawnWorkspaceTab() const
{
	return false;
}

UItemDefinitionBase* FStandaloneItemDefinitionEditorHost::GetItemDefinition() const
{
	if (const TSharedPtr<FItemDefinitionApplication> EditorPtr = WeakEditor.Pin())
	{
		return EditorPtr->GetItemDefinition();
	}

	return nullptr;
}

FSimpleMulticastDelegate& FStandaloneItemDefinitionEditorHost::OnItemDefinitionChanged()
{
	return OnItemDefinitionChangedDelegate;
}

TSharedPtr<IDetailsView> FStandaloneItemDefinitionEditorHost::GetAssetDetailsView() const
{
	if (const TSharedPtr<FItemDefinitionApplication> EditorPtr = WeakEditor.Pin())
	{
		return EditorPtr->GetAssetDetailsView();
	}

	return nullptr;
}

TSharedPtr<IDetailsView> FStandaloneItemDefinitionEditorHost::GetDetailsView() const
{
	if (const TSharedPtr<FItemDefinitionApplication> EditorPtr = WeakEditor.Pin())
	{
		return EditorPtr->GetSelectionDetailsView();
	}

	return nullptr;
}

TSharedPtr<UE::ItemizationEditor::FWorkspaceTabHost> FStandaloneItemDefinitionEditorHost::GetTabHost() const
{
	return TabHost;
}
