// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Toolkits/ItemizationEditorAppMode.h"

class FItemizationAppMode_Default : public FItemizationEditorAppMode
{
public:
	FItemizationAppMode_Default(TSharedPtr<FItemizationEditorApplication> InApp, const IItemizationCoreEditorModule::FItemizationAppModeArgs& InArgs);

protected:
	//~ Begin FItemizationEditorAppMode Interface
	virtual void PostActivateMode() override;
	//~ End FItemizationEditorAppMode Interface
	virtual bool CanShowMode() const override;
};