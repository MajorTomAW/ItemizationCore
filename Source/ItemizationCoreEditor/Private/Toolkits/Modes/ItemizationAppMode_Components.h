// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Toolkits/ItemizationEditorAppMode.h"

class FItemizationAppMode_Components : public FItemizationEditorAppMode
{
public:
	FItemizationAppMode_Components(TSharedPtr<FItemizationEditorApplication> InApp, const IItemizationCoreEditorModule::FItemizationAppModeArgs& InArgs);

protected:
	virtual void PostActivateMode() override;
};