// Copyright Epic Games, Inc. All Rights Reserved.


#include "ItemizationEditorCommands.h"

#include "Styles/ItemizationEditorStyle.h"

#define LOCTEXT_NAMESPACE "ItemizationEditor"

FItemizationEditorCommands::FItemizationEditorCommands()
	: TCommands
	(
		TEXT("ItemizationCore"),
		LOCTEXT("ItemizationCore", "Itemization Core Plugin"),
		NAME_None,
		FItemizationEditorStyle::Get()->GetStyleSetName()
	)
{
}

void FItemizationEditorCommands::RegisterCommands()
{
	UI_COMMAND(
		AppMode_Default,
		"Default Mode",
		"Switches to the default editor mode",
		EUserInterfaceActionType::None,
		FInputChord(EModifierKey::Shift, EKeys::One));

	UI_COMMAND(
		AppMode_Components,
		"Components Mode",
		"Switches to the components editor mode",
		EUserInterfaceActionType::None,
		FInputChord(EModifierKey::Shift, EKeys::Two));

	UI_COMMAND(
		AppMode_Equipment,
		"Equipment Mode",
		"Switches to the equipment editor mode",
		EUserInterfaceActionType::None,
		FInputChord(EModifierKey::Shift, EKeys::Three));
}

#undef LOCTEXT_NAMESPACE
