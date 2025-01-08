// Copyright © 2024 Playton. All Rights Reserved.


#include "FeatureActions/GameFeatureAction_AddItems.h"

#include "ItemDefinition.h"
#include "ItemizationCoreLog.h"
#include "ActorComponents/InventoryManager.h"

#if WITH_EDITOR
#include "Misc/DataValidation.h"
#endif

const FName UGameFeatureAction_AddItems::NAME_ItemAddReady("ItemAddReady");

#define LOCTEXT_NAMESPACE "GameFeatures"

UGameFeatureAction_AddItems::UGameFeatureAction_AddItems()
{
}

#if WITH_GAME_FEATURES
void UGameFeatureAction_AddItems::OnGameFeatureActivating(FGameFeatureActivatingContext& Context)
{
	FPerContextData& ActiveData = ContextData.FindOrAdd(Context);

	if (!ensureAlways(ActiveData.ActiveExtensions.IsEmpty()) ||
		!ensureAlways(ActiveData.ComponentRequests.IsEmpty()))
	{
		Reset(ActiveData);
	}
	
	Super::OnGameFeatureActivating(Context);
}

void UGameFeatureAction_AddItems::OnGameFeatureDeactivating(FGameFeatureDeactivatingContext& Context)
{
	Super::OnGameFeatureDeactivating(Context);

	FPerContextData* ActiveData = ContextData.Find(Context);
	if (ensure(ActiveData))
	{
		Reset(*ActiveData);
	}
}
#endif

#if WITH_EDITOR && WITH_GAME_FEATURES
EDataValidationResult UGameFeatureAction_AddItems::IsDataValid(class FDataValidationContext& Context) const
{
	EDataValidationResult Result = Super::IsDataValid(Context);

	int32 Index = 0;
	for (const auto& Entry : ItemsList)
	{
		if (Entry.ActorClass.IsNull())
		{
			Result = EDataValidationResult::Invalid;
			Context.AddError(FText::Format(LOCTEXT("EntryHasNullItemDefinition", "Null ActorClass at index {0} in ItemsList"), FText::AsNumber(Index)));
		}

		int32 ItemIndex = 0;
		for (const auto& Item : Entry.GrantedItems)
		{
			if (Item.ItemDefinition.IsNull())
			{
				Result = EDataValidationResult::Invalid;
				Context.AddError(FText::Format(LOCTEXT("EntryHasNullItemDefinition", "Null ItemDefinition at index {0} in ItemsList[{1}].GrantedItems"), FText::AsNumber(ItemIndex), FText::AsNumber(Index)));
			}

			++ItemIndex;
		}

		++Index;
	}

	return Result;
}
#endif

#if WITH_GAME_FEATURES
void UGameFeatureAction_AddItems::OnAddToWorld(
	const FWorldContext& WorldContext, const FGameFeatureStateChangeContext& ChangeContext)
{
	UWorld* World = WorldContext.World();
	UGameInstance* GameInstance = WorldContext.OwningGameInstance;
	FPerContextData& ActiveData = ContextData.FindOrAdd(ChangeContext);

	if ((GameInstance != nullptr) &&
		(World != nullptr) &&
		World->IsGameWorld())
	{
		if (UGameFrameworkComponentManager* Manager = GameInstance->GetSubsystem<UGameFrameworkComponentManager>())
		{
			int32 EntryIndex = 0;
			for (const auto& Entry : ItemsList)
			{
				if (Entry.ActorClass.IsNull())
				{
					continue;
				}

				UGameFrameworkComponentManager::FExtensionHandlerDelegate AddItemsDelegate =
					UGameFrameworkComponentManager::FExtensionHandlerDelegate::CreateUObject(this, &ThisClass::HandleActorExtension, EntryIndex, ChangeContext);

				TSharedPtr<FComponentRequestHandle> RequestHandle = Manager->AddExtensionHandler(Entry.ActorClass, AddItemsDelegate);
				ActiveData.ComponentRequests.Add(RequestHandle);
				EntryIndex++;
			}
		}
	}
}

void UGameFeatureAction_AddItems::Reset(FPerContextData& ActiveData)
{
	while (!ActiveData.ActiveExtensions.IsEmpty())
	{
		auto It = ActiveData.ActiveExtensions.CreateIterator();
		RemoveActorItems(It->Key, ActiveData);
	}

	ActiveData.ComponentRequests.Empty();
}

void UGameFeatureAction_AddItems::HandleActorExtension(
	AActor* Actor, FName EventName, int32 EntryIndex, FGameFeatureStateChangeContext ChangeContext)
{
	FPerContextData* ActiveData = ContextData.Find(ChangeContext);
	if (ItemsList.IsValidIndex(EntryIndex) && ActiveData)
	{
		const FGameFeatureItemEntry& Entry = ItemsList[EntryIndex];
		
		if ((EventName == UGameFrameworkComponentManager::NAME_ExtensionRemoved) ||
			(EventName == UGameFrameworkComponentManager::NAME_ReceiverRemoved))
		{
			RemoveActorItems(Actor, *ActiveData);
		}
		else if ((EventName == UGameFrameworkComponentManager::NAME_ExtensionAdded) ||
			(EventName == NAME_ItemAddReady))
		{
			AddActorItems(Actor, Entry, *ActiveData);
		}
	}
}

void UGameFeatureAction_AddItems::AddActorItems(
	AActor* Actor, const FGameFeatureItemEntry& ItemsEntry, FPerContextData& ActiveData)
{
	check(Actor);
	if (!Actor->HasAuthority())
	{
		return;
	}

	// Early out if Actor already has item extensions applied
	if (ActiveData.ActiveExtensions.Find(Actor) != nullptr)
	{
		return;
	}

	if (UInventoryManager* InventoryManager = FindOrAddComponentForActor<UInventoryManager>(Actor, ItemsEntry, ActiveData))
	{
		FActorExtensions AddedExtensions;
		AddedExtensions.Items.Reserve(ItemsEntry.GrantedItems.Num());

		for (const auto& Item : ItemsEntry.GrantedItems)
		{
			if (Item.ItemDefinition.IsNull())
			{
				continue;
			}

			
			FInventoryItemEntry NewItemEntry(Item.ItemDefinition.LoadSynchronous(), Item.Quantity, Actor);
			FItemActionContextData ItemActionCOntext = InventoryManager->CreateItemActionContextData(NewItemEntry);
			ItemActionCOntext.ContextTags->AddTag(FGameplayTag());
			
			FInventoryItemEntryHandle NewHandle = InventoryManager->GiveItem(NewItemEntry, ItemActionCOntext);
			AddedExtensions.Items.Add(NewHandle);
		}
	}
	else
	{
		ITEMIZATION_LOG(Error, TEXT("Failed to find/add an item component to '%s'. Items will not be granted."), *Actor->GetPathName());
	}
}

void UGameFeatureAction_AddItems::RemoveActorItems(AActor* Actor, FPerContextData& ActiveData)
{
	if (FActorExtensions* ActorExtensions = ActiveData.ActiveExtensions.Find(Actor))
	{
		if (UInventoryManager* InventoryManager = UInventoryManager::GetInventoryManager(Actor))
		{
			for (FInventoryItemEntryHandle Handle : ActorExtensions->Items)
			{
				InventoryManager->RemoveItem(Handle);
			}
		}

		ActiveData.ActiveExtensions.Remove(Actor);
	}
}

UActorComponent* UGameFeatureAction_AddItems::FindOrAddComponentForActor(
	UClass* ComponentType, AActor* Actor, const FGameFeatureItemEntry& ItemsEntry, FPerContextData& ActiveData)
{
	UActorComponent* Component = Actor->FindComponentByClass(ComponentType);

	bool bMakeComponentRequest = (Component == nullptr);
	if (Component)
	{
		// Check to see if this component was created from a different `UGameFrameworkComponentManager` request.
		// `Native` is what `CreationMethod` defaults to for dynamically added components.
		if (Component->CreationMethod == EComponentCreationMethod::Native)
		{
			// Attempt to tell the difference between a true native component and one created by the GameFrameworkComponent system.
			// If it is from the UGameFrameworkComponentManager, then we need to make another request (requests are ref counted).
			UObject* ComponentArchetype = Component->GetArchetype();
			bMakeComponentRequest = ComponentArchetype->HasAnyFlags(RF_ClassDefaultObject);
		}
	}

	if (bMakeComponentRequest)
	{
		UWorld* World = Actor->GetWorld();
		UGameInstance* GameInstance = World->GetGameInstance();

		if (UGameFrameworkComponentManager* ComponentMan = UGameInstance::GetSubsystem<UGameFrameworkComponentManager>(GameInstance))
		{
			TSharedPtr<FComponentRequestHandle> RequestHandle = ComponentMan->AddComponentRequest(ItemsEntry.ActorClass, ComponentType);
			ActiveData.ComponentRequests.Add(RequestHandle);
		}

		if (!Component)
		{
			Component = Actor->FindComponentByClass(ComponentType);
			ensureAlways(Component);
		}
	}

	return Component;
}
#endif

#undef LOCTEXT_NAMESPACE