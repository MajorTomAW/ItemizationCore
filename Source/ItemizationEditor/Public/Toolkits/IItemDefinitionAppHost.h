// Author: Tom Werner (MajorT), 2025 November

#pragma once

#include "UObject/Object.h"

#include "IItemDefinitionAppHost.generated.h"

class UItemDefinitionBase;

namespace UE::ItemizationEditor
{
	class FWorkspaceTabHost;
}

/** Interface required for cross-referencing ItemDefinitionApp modes across different AssetEditors. */
class IItemDefinitionAppHost : public TSharedFromThis<IItemDefinitionAppHost>
{
public:
	IItemDefinitionAppHost() = default;
	virtual ~IItemDefinitionAppHost() = default;

	virtual bool CanToolkitSpawnWorkspaceTab() const = 0;

	/** Returns the item definition being edited by this host. */
	virtual UItemDefinitionBase* GetItemDefinition() const = 0;

	/** Simple delegate that is called by the editor whenever the Item Definition changes. */
	virtual FSimpleMulticastDelegate& OnItemDefinitionChanged() = 0;

	/** Details view accessors. */
	virtual TSharedPtr<IDetailsView> GetAssetDetailsView() const = 0;

	/** Returns the host object that hosted the editor tabs. */
	virtual TSharedPtr<UE::ItemizationEditor::FWorkspaceTabHost> GetTabHost() const = 0;
};

UCLASS()
class ITEMIZATIONEDITOR_API UItemDefinitionAppContext : public UObject
{
	GENERATED_BODY()

public:
	TSharedPtr<IItemDefinitionAppHost> AppHostInterface;
};
