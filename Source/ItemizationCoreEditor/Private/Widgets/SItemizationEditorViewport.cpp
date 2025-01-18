// Copyright Epic Games, Inc. All Rights Reserved.


#include "Widgets/SItemizationEditorViewport.h"

#include "AdvancedPreviewScene.h"
#include "ItemDefinition.h"
#include "SlateOptMacros.h"


FItemizationEditorViewportClient::FItemizationEditorViewportClient(FPreviewScene* InPreviewScene, const TWeakPtr<SEditorViewport>& InEditorViewport)
	: FEditorViewportClient(nullptr, InPreviewScene, InEditorViewport)
{
	EngineShowFlags.DisableAdvancedFeatures();
	EngineShowFlags.SetSnap(true);
	EngineShowFlags.SetCompositeEditorPrimitives(true);
	EngineShowFlags.SetSelection(true);
	EngineShowFlags.SetSelectionOutline(true);

	DrawHelper.bDrawGrid = true;
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
		CleanUpScene(PreviewScene);
	}

	if (Client.IsValid())
	{
		Client->Invalidate();
		Client->Viewport = nullptr;
	}
}

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION
void SItemizationEditorViewport::Construct(const FArguments& InArgs)
{
	PreviewScene = MakeShareable(new FAdvancedPreviewScene(FPreviewScene::ConstructionValues()));
	PreviewScene->SetFloorVisibility(true);
	PreviewScene->SetFloorOffset(5.f);
	PreviewScene->SetEnvironmentVisibility(true);
	PreviewScene->SetLightDirection(FRotator(-40.f, 128.f, 0.f));

	SEditorViewport::FArguments ViewportArgs = InArgs._ViewportArgs;
	ViewportArgs.IsEnabled(FSlateApplication::Get().GetNormalExecutionAttribute());
	SEditorViewport::Construct(ViewportArgs);
}
END_SLATE_FUNCTION_BUILD_OPTIMIZATION

void SItemizationEditorViewport::AddReferencedObjects(FReferenceCollector& Collector)
{
	Collector.AddReferencedObjects(SceneComponents);
}

TSharedRef<FEditorViewportClient> SItemizationEditorViewport::MakeEditorViewportClient()
{
	if (!ViewportClient.IsValid())
	{
		ViewportClient = MakeShared<FItemizationEditorViewportClient>(PreviewScene.Get(), SharedThis(this));
		ViewportClient->ViewportType = LVT_Perspective;
		ViewportClient->SetViewLocation(EditorViewportDefs::DefaultPerspectiveViewLocation);
		ViewportClient->SetViewRotation(EditorViewportDefs::DefaultPerspectiveViewRotation);
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


void SItemizationEditorViewport::UpdateViewport(UItemDefinition* OwningItem, bool bForceRebuild)
{
	if (!PreviewScene.IsValid())
	{
		return;
	}

	// Check if we have to rebuild the scene
	if (NeedsToRebuildScene(OwningItem, PreviewScene) || bForceRebuild)
	{
		RebuildScene(OwningItem, PreviewScene);
		return;
	}

	// Nothing to do
	if (SceneComponents.IsEmpty())
	{
		return;
	}

	// Update the scene
	auto FindRoot = [this, OwningItem](USceneComponent* Comp)->bool
	{
		if (Comp == nullptr)
		{
			return false;
		}
		
		// Root Actor
		if (Comp->GetOwner()->GetClass() == AActor::StaticClass()) //@TODO: root actor
		{
			return true;
		}

		// Source Actor
		if (Comp->GetOwner()->GetClass() == OwningItem->SourceActorBlueprint.LoadSynchronous())
		{
			return true;
		}
		
		return false;
	};

	if (SceneComponents.Num() == 0)
	{
		return;
	}
	
	USceneComponent* RootComp = *SceneComponents.FindByPredicate(FindRoot); 
	if (RootComp == nullptr)
	{
		return;
	}

	const FTransform NewTransform = FTransform(OwningItem->DefaultRotation, OwningItem->DefaultLocation);
	RootComp->SetWorldTransform(NewTransform);
}

bool SItemizationEditorViewport::HasAnyViewportData(const UItemDefinition* OwningItem) const
{
	if (OwningItem == nullptr)
	{
		return false;
	}

	const bool bHasSourceActor = !OwningItem->SourceActorBlueprint.IsNull();
	const bool bHasData = OwningItem->ActorDataList.Num() > 0;
	return bHasSourceActor || bHasData;
}

bool SItemizationEditorViewport::SceneHasAnyData(TSharedPtr<FAdvancedPreviewScene> InScene) const
{
	if (!ensure(InScene))
	{
		return false;
	}

	if (SceneComponents.IsEmpty())
	{
		return false;
	}

	return true;
}

bool SItemizationEditorViewport::NeedsToRebuildScene(
	const UItemDefinition* OwningItem, TSharedPtr<FAdvancedPreviewScene> InScene) const
{
	if (!ensure(InScene))
	{
		return false;
	}

	if (!HasAnyViewportData(OwningItem) && !SceneHasAnyData(InScene))
	{
		return false;
	}

	// Check if the source actor has changed
	if (OwningItem->SourceActorBlueprint.IsNull())
	{
		if (LastSourceBP.IsValid()) // Source null but we have a valid last source
		{
			return true;
		}
	}
	else
	{
		if (!LastSourceBP.IsValid()) // Source valid but we have a null last source
		{
			//UE_LOG(LogTemp, Warning, TEXT("Source valid but we have a null last source"));
			return true;
		}

		if (LastSourceBP.Get() != OwningItem->SourceActorBlueprint.LoadSynchronous()) // Source changed
		{
			//UE_LOG(LogTemp, Warning, TEXT("Source changed"));
			return true;
		}
	}

	// Size mismatch
	if (SceneComponents.Num() - 1 /* don't count the root */ != OwningItem->ActorDataList.Num())
	{
		//UE_LOG(LogTemp, Warning, TEXT("Size mismatch. %d != %d"), PreviewComponents.Num() - 1, OwningItem->ActorData.Num());
		return true;
	}

	// Maybe check if the individual components have changed
	return false;
}

void SItemizationEditorViewport::RebuildScene(
	const UItemDefinition* OwningItem, TSharedPtr<FAdvancedPreviewScene> InScene)
{
	if (SceneHasAnyData(InScene))
	{
		CleanUpScene(InScene);
	}

	// After cleaning scene, check if we have any data to show
	if (!HasAnyViewportData(OwningItem))
	{
		return;
	}

	//UE_LOG(LogTemp, Error, TEXT("Rebuilding scene"));
	if (OwningItem->SourceActorBlueprint.IsNull())
	{
		bool Invalid = true;
		for (const auto& DataList : OwningItem->ActorDataList)
		{
			if (!DataList.ActorBlueprint.IsNull())
			{
				Invalid = false;
			}
		}

		if (Invalid)
		{
			return;
		}
	}

	FActorSpawnParameters SpawnParams;
	{
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		SpawnParams.ObjectFlags = RF_Transient | RF_Transactional;
	}

	FTransform RootTransform = FTransform(OwningItem->DefaultRotation, OwningItem->DefaultLocation);

	AActor* RootActor = nullptr;
	if (!OwningItem->SourceActorBlueprint.IsNull())
	{
		OwningItem->SourceActorBlueprint.LoadSynchronous();
		AActor* CDO = OwningItem->SourceActorBlueprint->GetDefaultObject<AActor>();
		if (!CDO->IsUnreachable())
		{
			RootActor = PreviewScene->GetWorld()->SpawnActor<AActor>(OwningItem->SourceActorBlueprint.Get(), FTransform::Identity, SpawnParams);
			LastSourceBP = OwningItem->SourceActorBlueprint.Get();	
		}
	}
	
	if (!RootActor)
	{
		RootActor = PreviewScene->GetWorld()->SpawnActor<AActor>(AActor::StaticClass(), FTransform::Identity, SpawnParams);
	}
	check(RootActor);

	if (RootActor->GetRootComponent() && !RootActor->GetRootComponent()->IsUnreachable())
	{
		SceneComponents.Add(RootActor->GetRootComponent());
		PreviewScene->AddComponent(RootActor->GetRootComponent(), RootTransform);
	}

	auto SpawnActor = [this, SpawnParams, RootActor](UClass* Class, const FTransform& SpawnTransform)->AActor*
	{
		if (Class == nullptr)
		{
			return nullptr;
		}

		// Check if the class can be spawned
		if (!Class->IsChildOf(AActor::StaticClass()))
		{
			return nullptr;
		}
		
		// Spawn the actor
		AActor* Spawned = PreviewScene->GetWorld()->SpawnActor<AActor>(Class, SpawnTransform, SpawnParams);
		check(Spawned);
		
		Spawned->AttachToActor(RootActor, FAttachmentTransformRules::KeepRelativeTransform);
		SceneComponents.Add(Spawned->GetRootComponent());
		return Spawned;
	};

	for (const FItemActorData& Data : OwningItem->ActorDataList)
	{
		if (Data.ActorBlueprint.IsNull())
		{
			continue;
		}
		
		UClass* Class = Data.ActorBlueprint.LoadSynchronous();
		if (Class == nullptr)
		{
			continue;
		}

		if (Class->IsUnreachable())
		{
			continue;
		}

		if (Class->IsNative())
		{
			continue;
		}
		
		const AActor* Spawned = SpawnActor(Class, Data.RelativeTransform);
		PreviewScene->AddComponent(Spawned->GetRootComponent(), Data.RelativeTransform);
	}
}

void SItemizationEditorViewport::CleanUpScene(TSharedPtr<FAdvancedPreviewScene> InScene)
{
	if (!InScene.IsValid())
	{
		return;
	}

	for (auto It = SceneComponents.CreateIterator(); It; ++It)
	{
		USceneComponent* Component = *It;
		if (!IsValid(Component))
		{
			continue;
		}

		TArray<USceneComponent*> Children;
		Component->GetChildrenComponents(true, Children);

		//UE_LOG(LogTemp, Display, TEXT("		Removing component %s"), *Component->GetName());
		for (USceneComponent* Child : Children)
		{
			if (!IsValid(Child))
			{
				continue;
			}
			InScene->RemoveComponent(Child);
		}
		
		InScene->RemoveComponent(Component);
	}

	SceneComponents.Reset();
}