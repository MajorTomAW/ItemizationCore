// Copyright Epic Games, Inc. All Rights Reserved.


#include "Widgets/SItemizationEditorViewport.h"

#include "AdvancedPreviewScene.h"
#include "ItemDefinition.h"
#include "ItemizationCoreEditor.h"
#include "ItemizationCoreEditorHelpers.h"
#include "SlateOptMacros.h"


FItemizationEditorViewportClient::FItemizationEditorViewportClient(FPreviewScene* InPreviewScene, const TWeakPtr<SEditorViewport>& InEditorViewport)
	: FEditorViewportClient(nullptr, InPreviewScene, InEditorViewport)
{
	EngineShowFlags.DisableAdvancedFeatures();
	EngineShowFlags.SetSnap(true);
	EngineShowFlags.SetCompositeEditorPrimitives(true);
	EngineShowFlags.SetSelection(true);
	EngineShowFlags.SetSelectionOutline(true);

	DrawHelper.bDrawGrid = false;
	DrawHelper.GridColorAxis = FColor(80,80,80);
	DrawHelper.GridColorMajor = FColor(22,22,22);
	DrawHelper.GridColorMinor = FColor(64,64,64);
	
	DrawHelper.bDrawWorldBox = false;
	DrawHelper.bDrawPivot = false;

	bUsingOrbitCamera = true;
	bSetListenerPosition = false;
	SetRealtime(true);
	SetGameView(false);
	OverrideNearClipPlane(1.f);

	ExposureSettings.bFixed = true;
	ExposureSettings.FixedEV100 = INDEX_NONE;
}

SItemizationEditorViewport::~SItemizationEditorViewport()
{
	if (PreviewScene.IsValid())
	{
	}

	if (Client.IsValid())
	{
		Client->Invalidate();
		Client->Viewport = nullptr;
	}
}

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION
void SItemizationEditorViewport::Construct(const FArguments& InArgs, const UItemDefinition* InOwningItem)
{
	PreviewScene = MakeShareable(new FAdvancedPreviewScene(FPreviewScene::ConstructionValues()));
	PreviewScene->SetFloorVisibility(true);
	PreviewScene->SetFloorOffset(5.f);
	PreviewScene->SetEnvironmentVisibility(true);
	PreviewScene->SetLightDirection(FRotator(-40.f, 128.f, 0.f));

	OwningItem = InOwningItem;

	SourceComponent = NewObject<UStaticMeshComponent>();
	PreviewScene->AddComponent(SourceComponent, FTransform::Identity);

	SEditorViewport::FArguments ViewportArgs = InArgs._ViewportArgs;
	ViewportArgs.IsEnabled(FSlateApplication::Get().GetNormalExecutionAttribute());
	SEditorViewport::Construct(ViewportArgs);

	bViewportDirty = true;
}
END_SLATE_FUNCTION_BUILD_OPTIMIZATION

void SItemizationEditorViewport::AddReferencedObjects(FReferenceCollector& Collector)
{
	Collector.AddReferencedObject(SourceComponent);
}

TSharedRef<FEditorViewportClient> SItemizationEditorViewport::MakeEditorViewportClient()
{
	if (!ViewportClient.IsValid())
	{
		ViewportClient = MakeShared<FItemizationEditorViewportClient>(PreviewScene.Get(), SharedThis(this));
		ViewportClient->ViewportType = LVT_Perspective;
		ViewportClient->SetViewLocation(EditorViewportDefs::DefaultPerspectiveViewLocation / 20.f);
		ViewportClient->SetViewRotation(EditorViewportDefs::DefaultPerspectiveViewRotation );
	}

	return ViewportClient.ToSharedRef();
}

TSharedPtr<SWidget> SItemizationEditorViewport::MakeViewportToolbar()
{
	return SAssetEditorViewport::MakeViewportToolbar();
}

void SItemizationEditorViewport::PopulateViewportOverlays(TSharedRef<SOverlay> Overlay)
{
}

void SItemizationEditorViewport::Tick(const FGeometry& AllottedGeometry, const double InCurrentTime,
	const float InDeltaTime)
{
	SAssetEditorViewport::Tick(AllottedGeometry, InCurrentTime, InDeltaTime);

	if (bViewportDirty)
	{
		bViewportDirty = false;
		UpdateViewport();
	}
}


void SItemizationEditorViewport::UpdateViewport(bool bForceRebuild)
{
	if (OwningItem == nullptr)
	{
		return;
	}

	UStaticMesh* Preview = IItemizationCoreEditorModule::Get().GetAssetConfig(OwningItem->GetClass())->FindPreviewMesh(OwningItem);

	if (Preview)
	{
		SetPreview(Preview);
	}
}

void SItemizationEditorViewport::SetPreview(UStaticMesh* InPreviewMesh)
{
	SourceComponent->SetStaticMesh(InPreviewMesh);

	// Update camera to show the preview component
	const FBoxSphereBounds Bounds = SourceComponent->Bounds;
	ViewportClient->FocusViewportOnBox(Bounds.GetBox(), true);
	ViewportClient->Invalidate();
}
