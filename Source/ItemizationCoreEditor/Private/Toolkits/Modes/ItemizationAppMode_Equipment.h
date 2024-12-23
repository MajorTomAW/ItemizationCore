// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Toolkits/ItemizationEditorAppMode.h"


class FItemizationAppMode_Equipment : public FItemizationEditorAppMode
{
public:
	FItemizationAppMode_Equipment(TSharedPtr<FItemizationEditorApplication> InApp, const IItemizationCoreEditorModule::FItemizationAppModeArgs& InArgs);

	//~ Begin FItemizationEditorAppMode Interface
	virtual bool CanActivateMode() const override;
	virtual bool CanShowMode() const override;
	virtual void PostActivateMode() override;
	//~ End FItemizationEditorAppMode Interface
};
