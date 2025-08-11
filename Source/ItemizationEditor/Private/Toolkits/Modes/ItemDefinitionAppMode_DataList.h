// Author: Tom Werner (MajorT), 2025

#pragma once

#include "Toolkits/ItemDefinitionApplicationMode.h"

class FItemDefinitionAppMode_DataList : public FItemDefinitionApplicationMode
{
public:
	FItemDefinitionAppMode_DataList(const TSharedPtr<IItemDefinitionApplication>& InApp);

	/** Name of this mode. */
	static const FName ModeId;

	//~ Begin FApplicationMode Interface
	virtual void PostActivateMode() override;
	//~ End FApplicationMode Interface
};
