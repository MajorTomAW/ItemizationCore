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
	void Construct(const FArguments& InArgs, const UItemDefinition* OwningItem);
	
	void UpdateViewport(bool bForceRebuild = false);

protected:
	//~ Begin FGCObject Interface
	virtual FString GetReferencerName() const override { return TEXT("SItemizationEditorViewport"); }
	virtual void AddReferencedObjects(FReferenceCollector& Collector) override;
	//~ End FGCObject Interface

	//~ Begin SEditorViewport Interface
	virtual TSharedRef<FEditorViewportClient> MakeEditorViewportClient() override;
	virtual TSharedPtr<SWidget> MakeViewportToolbar() override;
	virtual void PopulateViewportOverlays(TSharedRef<SOverlay> Overlay) override;

	virtual void Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime) override;
	//~ End SEditorViewport Interface

	void SetPreview(UStaticMesh* InPreviewMesh);

private:
	TSharedPtr<FItemizationEditorViewportClient> ViewportClient;
	TObjectPtr<UStaticMeshComponent> SourceComponent;
	TSharedPtr<FAdvancedPreviewScene> PreviewScene;
	const UItemDefinition* OwningItem = nullptr;

	bool bViewportDirty = false;
};
