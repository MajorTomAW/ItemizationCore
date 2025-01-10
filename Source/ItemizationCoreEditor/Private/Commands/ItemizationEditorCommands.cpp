// Copyright Epic Games, Inc. All Rights Reserved.


#include "Commands/ItemizationEditorCommands.h"

#include "ItemizationCoreEditor.h"
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
	MappedCommands.Add("AppMode_Default", AppMode_Default);

	UI_COMMAND(
		AppMode_Components,
		"Components Mode",
		"Switches to the components editor mode",
		EUserInterfaceActionType::None,
		FInputChord(EModifierKey::Shift, EKeys::Two));
	MappedCommands.Add("AppMode_Components", AppMode_Components);

	UI_COMMAND(
		AppMode_Equipment,
		"Equipment Mode",
		"Switches to the equipment editor mode",
		EUserInterfaceActionType::None,
		FInputChord(EModifierKey::Shift, EKeys::Three));
	MappedCommands.Add("AppMode_Equipment", AppMode_Equipment);

	// Extra modes

	// Etc Mode A
	UI_COMMAND(
		AppMode_Extra_A,
		"Etc Mode A",
		"Switches to the extra editor mode A",
		EUserInterfaceActionType::None,
		FInputChord(EModifierKey::Shift, EKeys::Four));
	MappedCommands.Add("AppMode_Extra_A", AppMode_Extra_A);

	// Etc Mode B
	UI_COMMAND(
		AppMode_Extra_B,
		"Etc Mode B",
		"Switches to the extra editor mode B",
		EUserInterfaceActionType::None,
		FInputChord(EModifierKey::Shift, EKeys::Five));
	MappedCommands.Add("AppMode_Extra_B", AppMode_Extra_B);

	// Etc Mode C
	UI_COMMAND(
		AppMode_Extra_C,
		"Etc Mode C",
		"Switches to the extra editor mode C",
		EUserInterfaceActionType::None,
		FInputChord(EModifierKey::Shift, EKeys::Six));
	MappedCommands.Add("AppMode_Extra_C", AppMode_Extra_C);
}

#undef LOCTEXT_NAMESPACE
