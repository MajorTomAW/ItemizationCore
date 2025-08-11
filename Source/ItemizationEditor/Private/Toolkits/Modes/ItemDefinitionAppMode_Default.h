// Author: Tom Werner (MajorT), 2025

#pragma once
#include "Toolkits/ItemDefinitionApplicationMode.h"

class FItemDefinitionAppMode_Default : public FItemDefinitionApplicationMode
{
public:
	FItemDefinitionAppMode_Default(const TSharedPtr<IItemDefinitionApplication>& InApp);

	/** Name of this mode */
	static const FName ModeId;

	//~ Begin FApplicationMode Interface
	virtual void PostActivateMode() override;
	//~ End FApplicationMode Interface
};