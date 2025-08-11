// Author: Tom Werner (MajorT), 2025

#pragma once

#include "Toolkits/AssetEditorModeUILayer.h"

class FItemDefinitionEditorModeUILayer : public FAssetEditorModeUILayer
{
public:
	FItemDefinitionEditorModeUILayer(const IToolkitHost* InToolkitHost);

	//~ Begin FAssetEditorModeUILayer Interface
	virtual void OnToolkitHostingStarted(const TSharedRef<IToolkit>& Toolkit) override;
	virtual void OnToolkitHostingFinished(const TSharedRef<IToolkit>& Toolkit) override;
	virtual TSharedPtr<FWorkspaceItem> GetModeMenuCategory() const override;
	//~ End FAssetEditorModeUILayer Interface

	void SetModeMenuCategory(const TSharedPtr<FWorkspaceItem>& MenuCategoryIn);
	
protected:
	TSharedPtr<FWorkspaceItem> MenuCategory;
};
