// Author: Tom Werner (MajorT), 2025


#include "Actions/GameFeatureAction_Itemization.h"

#include "Components/GameFrameworkComponentManager.h"
#include "InventoryBase.h"


#include UE_INLINE_GENERATED_CPP_BY_NAME(GameFeatureAction_Itemization)

void UGameFeatureAction_Itemization::OnGameFeatureActivating(FGameFeatureActivatingContext& Context)
{
	// Bind to the game instance start delegate
	FItemizationData& Data = ItemizationData.FindOrAdd(Context);
	Data.GameInstanceStartDelegate = FWorldDelegates::OnStartGameInstance.AddUObject
	(
		this,
		&ThisClass::HandleGameInstanceStart,
		FGameFeatureStateChangeContext(Context)
	);

	// Add to any worlds that are already loaded
	for (const FWorldContext& WorldContext : GEngine->GetWorldContexts())
	{
		if (Context.ShouldApplyToWorldContext(WorldContext))
		{
			OnAddToWorld(WorldContext, Context);
		}
	}
}

void UGameFeatureAction_Itemization::OnGameFeatureDeactivating(FGameFeatureDeactivatingContext& Context)
{
	FItemizationData* Data = ItemizationData.Find(Context);
	if (ensure(Data))
	{
		FWorldDelegates::OnStartGameInstance.Remove(Data->GameInstanceStartDelegate);
	}
}

void UGameFeatureAction_Itemization::HandleGameInstanceStart(
	UGameInstance* GameInstance,
	FGameFeatureStateChangeContext ChangeContext)
{
	if (const FWorldContext* WorldContext = GameInstance->GetWorldContext())
	{
		if (ChangeContext.ShouldApplyToWorldContext(*WorldContext))
		{
			OnAddToWorld(*WorldContext, ChangeContext);
		}
	}
}

void UGameFeatureAction_Itemization::OnAddToWorld(
	const FWorldContext& WorldContext,
	const FGameFeatureStateChangeContext& ChangeContext)
{
	const UWorld* World = WorldContext.World();
	const UGameInstance* GameInstance = World->GetGameInstance();
	FItemizationData& ActiveData = ItemizationData.FindOrAdd(ChangeContext);

	if (IsValid(GameInstance)
		&& IsValid(World)
		&& World->IsGameWorld())
	{
		if (UGameFrameworkComponentManager* ComponentManager = GameInstance->GetSubsystem<UGameFrameworkComponentManager>())
		{
			TSoftClassPtr<AInventoryBase> InventoryClass = AInventoryBase::StaticClass();
			TSharedPtr<FComponentRequestHandle> ExtensionRequestHandle = ComponentManager->AddExtensionHandler
			(
				InventoryClass,
				UGameFrameworkComponentManager::FExtensionHandlerDelegate::CreateUObject(this, &ThisClass::HandleActorExtension, ChangeContext)
			);

			ActiveData.ComponentRequests.Add(ExtensionRequestHandle);
		}
	}
}

void UGameFeatureAction_Itemization::HandleActorExtension(
	AActor* Actor,
	FName EventName,
	FGameFeatureStateChangeContext ChangeContext)
{
	FItemizationData& ActiveData = ItemizationData.FindOrAdd(ChangeContext);
	AInventoryBase* Inventory = Cast<AInventoryBase>(Actor);

	if ((EventName == UGameFrameworkComponentManager::NAME_ExtensionRemoved) ||
		(EventName == UGameFrameworkComponentManager::NAME_ReceiverRemoved))
	{
		OnClearItemizationData(Inventory, Inventory->GetOwner(), ChangeContext);
	}
	else if ((EventName == UGameFrameworkComponentManager::NAME_ExtensionAdded) ||
		(EventName == UGameFrameworkComponentManager::NAME_ReceiverAdded))
	{
		OnInitItemizationData(Inventory, Inventory->GetOwner(), ChangeContext);
	}
}
