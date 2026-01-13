// Author: Tom Werner (MajorT), 2025 November


#include "ItemDefinitionAppCommands.h"

#define LOCTEXT_NAMESPACE "ItemDefinitionAppCommands"

FItemDefinitionAppCommands::FItemDefinitionAppCommands()
	: TCommands(
		TEXT("ItemDefinitionAppCommands"),
		LOCTEXT("Tooltip", "Item Definition App"),
		NAME_None,
		TEXT("ItemizationEditorStyle"))
{
}

void FItemDefinitionAppCommands::RegisterCommands()
{
	UI_COMMAND(DeleteItemData, "Delete", "Delete Selected Item Data", EUserInterfaceActionType::Button, FInputChord(EKeys::Platform_Delete));
}

#undef LOCTEXT_NAMESPACE
