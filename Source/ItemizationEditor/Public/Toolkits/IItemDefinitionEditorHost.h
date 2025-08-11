// Author: Tom Werner (MajorT), 2025

#pragma once

#include "UObject/Object.h"

#include "IItemDefinitionEditorHost.generated.h"

class UItemDefinitionBase;
class IDetailsView;

namespace UE::ItemizationEditor
{
	class FWorkspaceTabHost;
}

/** Interface required for cross-referencing ItemDefinitionEditor modes across different AssetEditors. */
class IItemDefinitionEditorHost : public TSharedFromThis<IItemDefinitionEditorHost>
{
public:
	IItemDefinitionEditorHost() = default;
	virtual ~IItemDefinitionEditorHost() = default;

	virtual bool CanToolkitSpawnWorkspaceTab() const = 0;

	/** Returns the Item Definition being edited by this host. */
	virtual UItemDefinitionBase* GetItemDefinition() const = 0;

	/** Simple delegate that is called by the editor whenever the Item Definition changes. */
	virtual FSimpleMulticastDelegate& OnItemDefinitionChanged() = 0;
	
	/** Details view accessors. */
	virtual TSharedPtr<IDetailsView> GetAssetDetailsView() const = 0;
	virtual TSharedPtr<IDetailsView> GetDetailsView() const = 0;

	/** Returns the host object that hosted the editor tabs. */
	virtual TSharedPtr<UE::ItemizationEditor::FWorkspaceTabHost> GetTabHost() const = 0;
};

UCLASS()
class ITEMIZATIONEDITOR_API UItemDefinitionEditorContext : public UObject
{
	GENERATED_BODY()

public:
	TSharedPtr<IItemDefinitionEditorHost> EditorHostInterface;
};