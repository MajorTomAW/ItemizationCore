// Copyright Epic Games, Inc. All Rights Reserved.


#include "ItemizationLevelEditorToolbar.h"

#include "Styles/ItemizationEditorStyle.h"

void FItemizationLevelEditorToolbar::RegisterItemizationLevelEditorToolBar()
{
	static bool bHasRegistered = false;
	if (!bHasRegistered)
	{
		bHasRegistered = true;

		RegisterAddItemMenu();
	}
	
	FToolMenuOwnerScoped MenuOwner("InventoryEditorToolBar");
	UToolMenu* Menu = UToolMenus::Get()->ExtendMenu("LevelEditor.LevelEditorToolBar.AssetsToolBar");
    
	FToolMenuSection& Section = Menu->FindOrAddSection("Content");
	FToolMenuEntry Entry = FToolMenuEntry::InitComboButton
		(
			"NewItem",
			FUIAction(),
			FOnGetContent::CreateStatic(&FItemizationLevelEditorToolbar::GenerateAddItemWidget),
			FText::FromString(TEXT("New Item")),
			FText::FromString(TEXT("Dummy")),
			FSlateIcon(FItemizationEditorStyle::Get()->GetStyleSetName(), "Icons.CreateItem.Background", NAME_None, "Icons.CreateItem.Overlay")
		);

	Entry.StyleNameOverride = "CalloutToolbar";
	Section.AddEntry(Entry);
}

TSharedRef<SWidget> FItemizationLevelEditorToolbar::GenerateAddItemWidget()
{
	FToolMenuContext MenuContext;
	return UToolMenus::Get()->GenerateWidget("LevelEditor.InventoryEditorToolBar.AddItem", MenuContext);
}

void FItemizationLevelEditorToolbar::RegisterAddItemMenu()
{
#define LOCTEXT_NAMESPACE "ItemizationLevelEditorToolBar"
	if (UToolMenus::Get()->IsMenuRegistered("LevelEditor.InventoryEditorToolBar.AddItem"))
	{
		return;
	}

	UToolMenu* AddItemMenu = UToolMenus::Get()->RegisterMenu("LevelEditor.InventoryEditorToolBar.AddItem");
	{
		FToolMenuSection& Section = AddItemMenu->AddSection("New");
		Section.InitSection("New", LOCTEXT("New_Label", "New Item"), FToolMenuInsert(NAME_None, EToolMenuInsertType::First));

		TSharedRef<SWidget> NewItemOptions =
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.AutoHeight()
			.MaxHeight(400.f)
			[
				SNew(SBox)
				.WidthOverride(180.f)
				[
					SNew(STextBlock).Text(LOCTEXT("Dummy", "Dummy Text")) //Dummy text
				]
			];

		Section.AddEntry(FToolMenuEntry::InitWidget("InventoryLevelEditorToolBar", NewItemOptions, FText::GetEmpty(), true));
	}
#undef LOCTEXT_NAMESPACE
}
