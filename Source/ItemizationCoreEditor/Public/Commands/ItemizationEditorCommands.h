// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Framework/Commands/Commands.h"

class ITEMIZATIONCOREEDITOR_API FItemizationEditorCommands : public TCommands<FItemizationEditorCommands>
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
	TSharedPtr<FUICommandInfo> AppMode_Extra_A;
	TSharedPtr<FUICommandInfo> AppMode_Extra_B;
	TSharedPtr<FUICommandInfo> AppMode_Extra_C;

	TMap</*CommandId*/ FName, /*Command*/ TSharedPtr<FUICommandInfo>> MappedCommands;
};