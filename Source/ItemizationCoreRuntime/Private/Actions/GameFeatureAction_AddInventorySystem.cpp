// Author: Tom Werner (MajorT), 2025


#include "Actions/GameFeatureAction_AddInventorySystem.h"

#include "GameFeaturesSubsystemSettings.h"
#include "Components/GameFrameworkComponentManager.h"
#include "Engine/AssetManager.h"

#include "Inventory/InventorySystemConfiguration.h"
#include "Inventory/InventoryDefinition.h"
#include "Components/InventoryComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GameFeatureAction_AddInventorySystem)

FItemizationGameFeatureInventoryEntry::FItemizationGameFeatureInventoryEntry()
	: bAddForClients(false)
	, bAddForServer(true)
{
}



void UGameFeatureAction_AddInventorySystem::OnGameFeatureActivating(FGameFeatureActivatingContext& Context)
{
	FPerContextData& ActiveData = ContextData.FindOrAdd(Context);
	if (!ensure(ActiveData.ActorData.IsEmpty()))
	{
		Reset(ActiveData);
	}

	// Bind delegates
	ActiveData.GameInstanceStartHandle = FWorldDelegates::OnStartGameInstance.AddUObject(this,
		&ThisClass::HandleGameInstanceStart, FGameFeatureStateChangeContext(Context));

	// Early call, in case any worlds with an associated game instance are already initialized
	for (const FWorldContext& WorldContext : GEngine->GetWorldContexts())
	{
		if (Context.ShouldApplyToWorldContext(WorldContext))
		{
			AddToWorld(WorldContext, Context);
		}
	}
}

void UGameFeatureAction_AddInventorySystem::OnGameFeatureDeactivating(FGameFeatureDeactivatingContext& Context)
{
	Super::OnGameFeatureDeactivating(Context);
	
	FPerContextData* ActiveData = ContextData.Find(Context);
	if (ensure(ActiveData))
	{
		FWorldDelegates::OnStartGameInstance.Remove(ActiveData->GameInstanceStartHandle);
		Reset(*ActiveData);
	}
}

#if WITH_EDITORONLY_DATA
void UGameFeatureAction_AddInventorySystem::AddAdditionalAssetBundleData(FAssetBundleData& AssetBundleData)
{
	if (UAssetManager::IsInitialized())
	{
		for (const auto& Entry : InventoryList)
		{
			if (Entry.bAddForClients)
			{
				AssetBundleData.AddBundleAsset(UGameFeaturesSubsystemSettings::LoadStateClient,
					FTopLevelAssetPath(Entry.InventoryConfig.GetPath()));
			}

			if (Entry.bAddForServer)
			{
				AssetBundleData.AddBundleAsset(UGameFeaturesSubsystemSettings::LoadStateServer,
					FTopLevelAssetPath(Entry.InventoryConfig.GetPath()));
			}
		}
	}
}
#endif


#if WITH_EDITOR
EDataValidationResult UGameFeatureAction_AddInventorySystem::IsDataValid(class FDataValidationContext& Context) const
{
	return Super::IsDataValid(Context);
}
#endif

void UGameFeatureAction_AddInventorySystem::HandleGameInstanceStart(
	UGameInstance* GameInstance,
	FGameFeatureStateChangeContext ChangeContext)
{
	if (FWorldContext* WorldContext = GameInstance->GetWorldContext())
	{
		if (ChangeContext.ShouldApplyToWorldContext(*WorldContext))
		{
			FPerContextData* Data = ContextData.Find(ChangeContext);
			if (ensure(Data))
			{
				AddToWorld(*WorldContext, ChangeContext);
			}
		}
	}
}

void UGameFeatureAction_AddInventorySystem::Reset(FPerContextData& ActiveData)
{
	ActiveData.ComponentRequests.Empty();

	for (TPair<FObjectKey, FPerActorData>& Pair : ActiveData.ActorData)
	{
		// Do stuff
	}
	ActiveData.ActorData.Empty();
}

void UGameFeatureAction_AddInventorySystem::AddToWorld(
	const FWorldContext& WorldContext,
	const FGameFeatureStateChangeContext& ChangeContext)
{
	const UWorld* World = WorldContext.World();
	UGameInstance* GameInstance = WorldContext.OwningGameInstance;
	FPerContextData& ActiveData = ContextData.FindOrAdd(ChangeContext);

	if (!IsValid(GameInstance) || !IsValid(World))
	{
		return;
	}

	if (World->IsGameWorld())
	{
		if (UGameFrameworkComponentManager* ComponentManager =
			GameInstance->GetSubsystem<UGameFrameworkComponentManager>())
		{
			const ENetMode NetMode = World->GetNetMode();
			const bool bIsServer = NetMode != NM_Client;
			const bool bIsClient = NetMode != NM_DedicatedServer;

			auto AddComponentRequest = [&, ComponentManager]
				(const TSoftClassPtr<AActor>& ReceiverClass, const TSoftClassPtr<UActorComponent>& SoftClass)
			{
				ensure(!SoftClass.IsNull());

				if (TSubclassOf<UActorComponent> ComponentClass = SoftClass.LoadSynchronous())
				{
					return ComponentManager->AddComponentRequest(ReceiverClass, ComponentClass);
				}
				
				if (!SoftClass.IsNull())
				{
					UE_LOG(LogGameFeatures, Error,
					       TEXT("[GameFeatureData %s]: Failed to load inventory component class %s."),
					       *GetPathNameSafe(this), *SoftClass.ToString());
				}

				return TSharedPtr<FComponentRequestHandle>();
			};
			
			for (const auto& Entry : InventoryList)
			{
				const bool bShouldAddRequest =
					(bIsServer && Entry.bAddForServer) ||
					(bIsClient && Entry.bAddForClients);

				// Skip if we don't need to add this request
				if (bShouldAddRequest == false)
				{
					continue;
				}

				// We only handle player inventories, as another class handles AI inventories
				for (const FInventoryProperties& Prop : Entry.InventoryConfig->PlayerInventorySetup->InventoryList)
				{
					ActiveData.ComponentRequests.Add(
						AddComponentRequest(Entry.ActorClass, Prop.InventoryComponent));
				}
				for (const FEquippableInventoryProperties& Prop : Entry.InventoryConfig->PlayerInventorySetup->EquippableInventoryList)
				{
					ActiveData.ComponentRequests.Add(
						AddComponentRequest(Entry.ActorClass, Prop.InventoryComponent));
				}

				// Also add a component request handle to listen to
				TSharedPtr<FComponentRequestHandle> Request =
					ComponentManager->AddExtensionHandler(Entry.ActorClass,
						UGameFrameworkComponentManager::FExtensionHandlerDelegate::CreateUObject(
							this,
							&ThisClass::HandleActorExtension, ChangeContext));
				ActiveData.ComponentRequests.Add(Request);
			}
		}
	}
}



void UGameFeatureAction_AddInventorySystem::HandleActorExtension(
	AActor* Actor,
	FName EventName,
	FGameFeatureStateChangeContext ChangeContext)
{
	FPerContextData& ActiveData = ContextData.FindOrAdd(ChangeContext);
}