// Author: Tom Werner (MajorT), 2025 November

#pragma once

#include "Toolkits/Modes/ItemDefinitionAppMode.h"

class FItemDataUIInfo;

class FItemDefinitionAppMode_DataList : public FItemDefinitionAppMode
{
	using ThisClass = FItemDefinitionAppMode_DataList;
public:
	FItemDefinitionAppMode_DataList(const TSharedPtr<IItemDefinitionApp>& InApp);

	/** Name of this mode */
	static const FName ModeId;

	//~ Begin FApplicationMode Interface
	virtual void PostActivateMode() override;
	virtual void PreDeactivateMode() override;
	//~ End FApplicationMode Interface

protected:
	void HandleModelSelectionChanged(const TArray<TSharedPtr<FItemDataUIInfo>>& Selection);
	TSharedPtr<IStructureDetailsView> GetDataListSelectionView();

protected:
	TWeakObjectPtr<UItemDefinitionBase> CachedItemDefinition = nullptr;
};
