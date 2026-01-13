// Author: Tom Werner (MajorT), 2025 November

#pragma once

class FItemDefinitionAppCommands : public TCommands<FItemDefinitionAppCommands>
{
public:
	FItemDefinitionAppCommands();

	virtual void RegisterCommands() override;

public:
	TSharedPtr<FUICommandInfo> DeleteItemData;
};
