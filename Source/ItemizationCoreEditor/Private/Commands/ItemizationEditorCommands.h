// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Framework/Commands/Commands.h"

class FItemizationEditorCommands : public TCommands<FItemizationEditorCommands>
{
public:
	FItemizationEditorCommands();

	//~ Begin TCommands
	virtual void RegisterCommands() override;
	//~ End TCommands

public:
	TSharedPtr<FUICommandInfo> AppMode_Default;
	TSharedPtr<FUICommandInfo> AppMode_Components;
	TSharedPtr<FUICommandInfo> AppMode_Equipment;
};