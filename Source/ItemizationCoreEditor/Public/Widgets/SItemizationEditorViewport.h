// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "SAssetEditorViewport.h"
#include "Widgets/SCompoundWidget.h"
#include "EditorViewportClient.h"

class UItemDefinition;
class FAdvancedPreviewScene;

class FItemizationEditorViewportClient : public FEditorViewportClient
{
public:
	FItemizationEditorViewportClient(FPreviewScene* InPreviewScene, const TWeakPtr<SEditorViewport>& InEditorViewport);
};

class SItemizationEditorViewport : public SAssetEditorViewport, public FGCObject
{
public:
	virtual ~SItemizationEditorViewport() override;
	SLATE_BEGIN_ARGS(SItemizationEditorViewport)
		: _ViewportArgs(SEditorViewport::FArguments())
		{
		}
		SLATE_ARGUMENT(SEditorViewport::FArguments, ViewportArgs)
	SLATE_END_ARGS()

public:
	void Construct(const FArguments& InArgs);
	
	void UpdateViewport(UItemDefinition* OwningItem, bool bForceRebuild = false);
	bool HasAnyViewportData(const UItemDefinition* OwningItem) const;
	bool SceneHasAnyData(TSharedPtr<FAdvancedPreviewScene> InScene) const;
	bool NeedsToRebuildScene(const UItemDefinition* OwningItem, TSharedPtr<FAdvancedPreviewScene> InScene) const;
	void RebuildScene(const UItemDefinition* OwningItem, TSharedPtr<FAdvancedPreviewScene> InScene);
	void CleanUpScene(TSharedPtr<FAdvancedPreviewScene> InScene);

protected:
	//~ Begin FGCObject Interface
	virtual FString GetReferencerName() const override { return TEXT("SItemizationEditorViewport"); }
	virtual void AddReferencedObjects(FReferenceCollector& Collector) override;
	//~ End FGCObject Interface

	//~ Begin SEditorViewport Interface
	virtual TSharedRef<FEditorViewportClient> MakeEditorViewportClient() override;
	virtual TSharedPtr<SWidget> MakeViewportToolbar() override;
	virtual void PopulateViewportOverlays(TSharedRef<SOverlay> Overlay) override;
	//~ End SEditorViewport Interface

private:
	TWeakObjectPtr<UClass> LastSourceBP;
	TSharedPtr<FItemizationEditorViewportClient> ViewportClient;
	TArray<TObjectPtr<USceneComponent>> SceneComponents;
	TSharedPtr<FAdvancedPreviewScene> PreviewScene;
};
