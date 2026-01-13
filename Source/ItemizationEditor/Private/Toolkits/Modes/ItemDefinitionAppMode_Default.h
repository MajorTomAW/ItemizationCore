// Author: Tom Werner (MajorT), 2025 November

#pragma once

#include "Toolkits/Modes/ItemDefinitionAppMode.h"

class FItemDefinitionAppMode_Default : public FItemDefinitionAppMode
{
public:
	FItemDefinitionAppMode_Default(const TSharedPtr<IItemDefinitionApp>& InApp);

	/** Name of this mode */
	static const FName ModeId;

	//~ Begin FApplicationMode Interface
	virtual void PostActivateMode() override;
	//~ End FApplicationMode Interface
};
