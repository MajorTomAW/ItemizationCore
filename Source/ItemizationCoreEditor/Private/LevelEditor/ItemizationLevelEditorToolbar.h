// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

class FItemizationLevelEditorToolbar
{
public:
	static void RegisterItemizationLevelEditorToolBar();

private:
	static TSharedRef<SWidget> GenerateAddItemWidget();
	static void RegisterAddItemMenu();
};
