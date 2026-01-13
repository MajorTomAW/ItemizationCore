// Author: Tom Werner (MajorT), 2025 November

#pragma once

#include "CoreMinimal.h"
#include "SCommonEditorViewportToolbarBase.h"
#include "SEditorViewport.h"
#include "Widgets/SCompoundWidget.h"

class SItemDefinitionAppViewport;
class FAdvancedPreviewScene;

class FItemDefinitionAppViewportClient
	: public FEditorViewportClient
	, public TSharedFromThis<FItemDefinitionAppViewportClient>
{
public:
	FItemDefinitionAppViewportClient(FAdvancedPreviewScene& InPreviewScene, const TSharedRef<SItemDefinitionAppViewport>& InViewport);
	virtual ~FItemDefinitionAppViewportClient() override;

	//~ Begin FEditorViewportClient Interface
	virtual FLinearColor GetBackgroundColor() const override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void Draw(FViewport* Viewport,FCanvas* Canvas) override;
	virtual bool ShouldOrbitCamera() const override;
	virtual FSceneView* CalcSceneView(FSceneViewFamily* ViewFamily, const int32 ViewIndex = INDEX_NONE) override;
	virtual bool CanSetWidgetMode(UE::Widget::EWidgetMode NewMode) const override { return false; }
	virtual bool CanCycleWidgetMode() const override { return false; }
	//~ End FEditorViewportClient Interface

protected:
	TWeakPtr<SItemDefinitionAppViewport> WeakViewport;
	FAdvancedPreviewScene* AdvancedPreviewScene = nullptr;
};

class SItemDefinitionAppViewport
	: public SEditorViewport
	, public FGCObject
	, public ICommonEditorViewportToolbarInfoProvider
{
public:
	SLATE_BEGIN_ARGS(SItemDefinitionAppViewport)
		{
		}

	SLATE_END_ARGS()

public:
	void Construct(const FArguments& InArgs);

	//~ Begin FGCObject Interface
	virtual void AddReferencedObjects(FReferenceCollector& Collector) override;
	virtual FString GetReferencerName() const override { return TEXT("SItemDefinitionAppViewport"); }
	//~ End FGCObject Interface

	//~ Begin ICommonEditorViewportToolbarInfoProvider Interface
	virtual TSharedRef<class SEditorViewport> GetViewportWidget() override;
	virtual TSharedPtr<FExtender> GetExtenders() const override;
	virtual void OnFloatingButtonClicked() override;
	//~ End ICommonEditorViewportToolbarInfoProvider Interface

protected:
	//~ Begin SEditorViewport Interface
	virtual TSharedRef<FEditorViewportClient> MakeEditorViewportClient() override;
	virtual TSharedPtr<SWidget> MakeViewportToolbar() override;
	virtual TSharedPtr<SWidget> BuildViewportToolbar() override;
	virtual TSharedPtr<IPreviewProfileController> CreatePreviewProfileController() override;
	virtual EVisibility OnGetViewportContentVisibility() const override;
	virtual void BindCommands() override;
	virtual void OnFocusViewportToSelection() override;
	virtual void PopulateViewportOverlays(TSharedRef<class SOverlay> Overlay) override;
	//~ End SEditorViewport Interface

private:
	/** Preview Scene - uses advanced preview settings */
	TSharedPtr<class FAdvancedPreviewScene> AdvancedPreviewScene;

	/** Level viewport client. */
	TSharedPtr<class FItemDefinitionAppViewportClient> ItemViewportClient;
};
