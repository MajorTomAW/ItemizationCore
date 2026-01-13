// Author: Tom Werner (MajorT), 2025 November


#include "SItemDefinitionAppViewport.h"

#include "AdvancedPreviewScene.h"
#include "SlateOptMacros.h"
#include "UnrealEdGlobals.h"
#include "Editor/UnrealEdEngine.h"
#include "ThumbnailRendering/ThumbnailManager.h"


FItemDefinitionAppViewportClient::FItemDefinitionAppViewportClient(
	FAdvancedPreviewScene& InPreviewScene,
	const TSharedRef<SItemDefinitionAppViewport>& InViewport)
		: FEditorViewportClient(nullptr, &InPreviewScene, StaticCastSharedRef<SEditorViewport>(InViewport))
		, AdvancedPreviewScene(&InPreviewScene)
{
	WeakViewport = InViewport;

	DrawHelper.bDrawPivot = false;
	DrawHelper.bDrawWorldBox = false;
	DrawHelper.bDrawKillZ = false;
	DrawHelper.bDrawGrid = true;
	DrawHelper.PerspectiveGridSize = UE_OLD_HALF_WORLD_MAX1;
	ShowWidget(false);

	FEditorViewportClient::SetViewMode(VMI_Lit);

	EngineShowFlags.SetSnap(false);

	OverrideNearClipPlane(1.0f);

	bDrawAxesGame = true;
}

FItemDefinitionAppViewportClient::~FItemDefinitionAppViewportClient()
{
}

FLinearColor FItemDefinitionAppViewportClient::GetBackgroundColor() const
{
	if (AdvancedPreviewScene != nullptr)
	{
		return AdvancedPreviewScene->GetBackgroundColor();
	}

	FLinearColor BackgroundColor = FLinearColor::Black;
	return BackgroundColor;
}

void FItemDefinitionAppViewportClient::Tick(float DeltaSeconds)
{
	FEditorViewportClient::Tick(DeltaSeconds);

	PreviewScene->GetWorld()->Tick(LEVELTICK_All, DeltaSeconds);
}

void FItemDefinitionAppViewportClient::Draw(FViewport* InViewport, FCanvas* Canvas)
{
	FEditorViewportClient::Draw(InViewport, Canvas);
}

bool FItemDefinitionAppViewportClient::ShouldOrbitCamera() const
{
	return true;
}

FSceneView* FItemDefinitionAppViewportClient::CalcSceneView(FSceneViewFamily* ViewFamily, const int32 InViewIndex)
{
	FSceneView* SceneView = FEditorViewportClient::CalcSceneView(ViewFamily);
	FFinalPostProcessSettings::FCubemapEntry& CubemapEntry = *new(SceneView->FinalPostProcessSettings.ContributingCubemaps) FFinalPostProcessSettings::FCubemapEntry;
	CubemapEntry.AmbientCubemap = GUnrealEd->GetThumbnailManager()->AmbientCubemap;
	CubemapEntry.AmbientCubemapTintMulScaleValue = FLinearColor::White;
	return SceneView;
}

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION

void SItemDefinitionAppViewport::Construct(const FArguments& InArgs)
{
	AdvancedPreviewScene = MakeShareable(new FAdvancedPreviewScene(FPreviewScene::ConstructionValues()));
	AdvancedPreviewScene->SetFloorVisibility(false);

	float Pitch = -40.0;
	float Yaw = 128.0;
	float Roll = 0.0;
	AdvancedPreviewScene->SetLightDirection(FRotator(Pitch, Yaw, Roll));

	SEditorViewport::Construct(SEditorViewport::FArguments());
	Client->EngineShowFlags.SetGrid(true);
}
END_SLATE_FUNCTION_BUILD_OPTIMIZATION

void SItemDefinitionAppViewport::AddReferencedObjects(FReferenceCollector& Collector)
{
}

TSharedRef<class SEditorViewport> SItemDefinitionAppViewport::GetViewportWidget()
{
	return SharedThis(this);
}

TSharedPtr<FExtender> SItemDefinitionAppViewport::GetExtenders() const
{
	TSharedPtr<FExtender> Result(MakeShareable(new FExtender));
	return Result;
}

void SItemDefinitionAppViewport::OnFloatingButtonClicked()
{
}

TSharedRef<FEditorViewportClient> SItemDefinitionAppViewport::MakeEditorViewportClient()
{
	ItemViewportClient = MakeShared<FItemDefinitionAppViewportClient>(*AdvancedPreviewScene.Get(), SharedThis(this));
	ItemViewportClient->SetViewLocation(FVector::ZeroVector);
	ItemViewportClient->SetViewRotation( FRotator(-30.0f, 0.0f, 0.0f) );
	ItemViewportClient->SetViewLocationForOrbiting( FVector::ZeroVector, 200.0f );
	ItemViewportClient->bSetListenerPosition = false;
	ItemViewportClient->SetRealtime(true);
	ItemViewportClient->SetGameView(false);

	return ItemViewportClient.ToSharedRef();
}

TSharedPtr<SWidget> SItemDefinitionAppViewport::MakeViewportToolbar()
{
	return SNew(SBox);
}

TSharedPtr<SWidget> SItemDefinitionAppViewport::BuildViewportToolbar()
{
	return SEditorViewport::BuildViewportToolbar();
}

TSharedPtr<IPreviewProfileController> SItemDefinitionAppViewport::CreatePreviewProfileController()
{
	return SEditorViewport::CreatePreviewProfileController();
}

EVisibility SItemDefinitionAppViewport::OnGetViewportContentVisibility() const
{
	EVisibility BaseVisibility = SEditorViewport::OnGetViewportContentVisibility();
	if (BaseVisibility != EVisibility::Visible)
	{
		return BaseVisibility;
	}
	return IsVisible() ? EVisibility::Visible : EVisibility::Collapsed;
}

void SItemDefinitionAppViewport::BindCommands()
{
	SEditorViewport::BindCommands();
}

void SItemDefinitionAppViewport::OnFocusViewportToSelection()
{
	SEditorViewport::OnFocusViewportToSelection();
}

void SItemDefinitionAppViewport::PopulateViewportOverlays(TSharedRef<class SOverlay> Overlay)
{
}

