// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

class FItemizationEditorApplication;

class ITEMIZATIONCOREEDITOR_API FItemizationEditorToolbar : public TSharedFromThis<FItemizationEditorToolbar>
{
public:
	virtual ~FItemizationEditorToolbar() = default;
	FItemizationEditorToolbar(TSharedPtr<FItemizationEditorApplication> InApp)
		: WeakApp(InApp)
	{
	}

	void AddModesToolbar(TSharedPtr<FExtender> Extender);
	
protected:
	TSharedPtr<FItemizationEditorApplication> GetApp() const { return WeakApp.Pin(); }
	virtual void FillModesToolbar(FToolBarBuilder& ToolBarBuilder);

	bool Execute_IsModeActive(FName ModeName) const;
	void Execute_SetMode(FName ModeName) const;
	bool CanExecute_SetMode(FName ModeName) const;
	bool CanShow_SetMode(FName ModeName) const;

private:
	TWeakPtr<FItemizationEditorApplication> WeakApp;
};
