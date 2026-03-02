// Author: Tom Werner (MajorT), 2025


#include "Components/InventoryComponent.h"

#include "ItemizationCoreLogChannels.h"
#include "Inventory/InventoryBase.h"
#include "Inventory/InventoryConfigAsset.h"
#include "Inventory/SlottableInventory.h"
#include "Inventory/Operations/InventoryOp_PlaceItemInSlot.h"
#include "Items/InventoryItemInstance.h"
#include "Items/ItemDefinitionBase.h"

#include "Net/UnrealNetwork.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(InventoryComponent)

UInventoryComponent::UInventoryComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bAutoActivate = true;

	PrimaryComponentTick.bStartWithTickEnabled = false;
	PrimaryComponentTick.bCanEverTick = true;

	SetIsReplicatedByDefault(true);
	bReplicateUsingRegisteredSubObjectList = true;

	bWantsInitializeComponent = true;
	bShouldAcquireInventoryOnInitialize = true;
	bAttachInventoryToOwner = true;
}

void UInventoryComponent::GetOwnedGameplayTags(FGameplayTagContainer& TagContainer) const
{
}

AInventoryBase* UInventoryComponent::GetInventory() const
{
	if (InventoryHandle.IsValid())
	{
		return InventoryHandle.GetInventory();
	}

	return nullptr;
}

void UInventoryComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	FDoRepLifetimeParams SharedParams;
	SharedParams.bIsPushBased = true;

	DOREPLIFETIME_WITH_PARAMS_FAST(ThisClass, InventoryHandle, SharedParams);

	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
}

void UInventoryComponent::PostInitProperties()
{
	Super::PostInitProperties();
}

void UInventoryComponent::InitializeComponent()
{
	Super::InitializeComponent();

	UWorld const* World = GetWorld();
	if (!IsValid(World) || !World->IsGameWorld())
	{
		return;
	}

	// Only continue if we're a server
	if ((GetNetMode() != NM_Client) &&
		IsValid(this) &&
		bShouldAcquireInventoryOnInitialize)
	{
		// Make sure we're starting with a clean inventory
		if (InventoryHandle.IsValid())
		{
			ITEMIZATION_WARN_CONTEXT("Cleaning up old inventory handle [%s] for %s.",
				*GetNameSafe(InventoryHandle.GetInventory()), *GetNameSafe(GetOwner()));
			InventoryHandle.Reset();
		}

		ITEMIZATION_VERBOSE_CONTEXT("Acquiring inventory on initialize for %s.",
			*GetNameSafe(GetOwner()));

		CreateInventory();
	}
}

void UInventoryComponent::PostNetReceive()
{
	Super::PostNetReceive();
}

void UInventoryComponent::OnRegister()
{
	Super::OnRegister();
}

void UInventoryComponent::BeginPlay()
{
	Super::BeginPlay();
}

#if WITH_EDITOR
EDataValidationResult UInventoryComponent::IsDataValid(FDataValidationContext& Context) const
{
	return Super::IsDataValid(Context);
}
#endif

void UInventoryComponent::CallOrRegister_OnInventoryInitialized(FOnInventoryInitialized::FDelegate&& Delegate)
{
	if (AInventoryBase* Inventory = GetInventory())
	{
		Delegate.ExecuteIfBound(Inventory);
	}
	else
	{
		OnInventoryInitialized.Add(MoveTemp(Delegate));
	}
}

TScriptInterface<IInventoryItemInstanceInterface> UInventoryComponent::FindItemInstanceById(const FInventoryItemId& ItemId) const
{
	AInventoryBase* Inventory = GetInventory();
	if (!IsValid(Inventory))
	{
		return nullptr;
	}

	// Find the item instance in the inventory
	TScriptInterface<IInventoryItemInstanceInterface> ItemInstance = Inventory->FindItemInstanceById(ItemId);
	if (IsValid(ItemInstance.GetObject()))
	{
		return ItemInstance;
	}

	ITEMIZATION_WARN_CONTEXT("Could not find item instance with handle [%s] in inventory [%s] for %s.",
		*ItemId.ToString(), *GetNameSafe(Inventory), *GetNameSafe(GetOwner()));

	return nullptr;
}

TArray<TScriptInterface<IInventoryItemInstanceInterface>> UInventoryComponent::GetInventoryItems() const
{
	AInventoryBase* Inventory = GetInventory();
	if (!IsValid(Inventory))
	{
		return TArray<TScriptInterface<IInventoryItemInstanceInterface>>();
	}

	return Inventory->GetAllItemInstances();
}

/*TArray<TScriptInterface<IInventoryItemInstanceInterface>> UInventoryComponent::GetInventoryItemsInGroup(FGameplayTag Group) const
{
	AInventoryBase* Inventory = GetInventory();
	if (!IsValid(Inventory))
	{
		return {};
	}

	return Inventory->GetItemInstancesInGroup(Group);
}*/

/*TArray<TScriptInterface<IInventoryItemInstanceInterface>> UInventoryComponent::GetInventoryItemsInGroups(TArray<FGameplayTag> Groups) const
{
	TArray<TScriptInterface<IInventoryItemInstanceInterface>> Result;
	AInventoryBase* Inventory = GetInventory();
	if (!IsValid(Inventory))
	{
		return Result;
	}

	for (const FGameplayTag& GroupTag : Groups)
	{
		Result.Append(GetInventoryItemsInGroup(GroupTag));
	}

	return Result;
}*/

FInventoryItemId UInventoryComponent::GiveItem(
	const UItemDefinitionBase* ItemDefinition,
	int32 StackSize,
	UObject* SourceObject,
	FGameplayTag GroupTag,
	int32& OutNumCouldNotAdd)
{
	FInventoryItemId Result = FInventoryItemId::InvalidId;

	AInventoryBase* Inventory = GetInventory();
	if (!IsValid(Inventory))
	{
		ITEMIZATION_ERROR_CONTEXT("Cannot give item [%s] to inventory [%s] for %s. Inventory is invalid.",
			*GetNameSafe(ItemDefinition), *GetNameSafe(Inventory), *GetNameSafe(GetOwner()));
		return Result;
	}

	// Build our give item operation parameters
	FInventoryOp_GiveItem::FParams Params;
	Params.ItemDefinition = ItemDefinition;
	Params.NumItems = StackSize;
	Params.GroupTag = GroupTag;
	Params.SourceObject = SourceObject;

	// Actually give the item
	if (const TInventoryOpPtr<FInventoryOp_GiveItem> Op = Inventory->GiveItem(MoveTemp(Params)))
	{
		Result = Op->Result.ItemId;

		// Let us know about how many items could not be added
		OutNumCouldNotAdd = Op->Result.Excess;
	}

	return Result;
}

int32 UInventoryComponent::RemoveItemByDefinition(
	const UItemDefinitionBase* ItemDefinition,
	int32 NumRemove,
	FGameplayTag GroupTag)
{
	int32 Result = NumRemove;
	AInventoryBase* Inventory = GetInventory();
	if (!IsValid(Inventory))
	{
		ITEMIZATION_ERROR_CONTEXT("Cannot remove item [%s] from inventory [%s] for %s. Inventory is invalid.",
			*GetNameSafe(ItemDefinition), *GetNameSafe(Inventory), *GetNameSafe(GetOwner()));
		return Result;
	}

	// Build our remove item operation parameters
	FInventoryOp_RemoveItem::FParams Params;
	Params.ItemDefinition = ItemDefinition;
	Params.NumRemove = NumRemove;
	Params.GroupTag = GroupTag;

	// Actually remove the item
	if (const TInventoryOpPtr<FInventoryOp_RemoveItem> Op = Inventory->RemoveItem(MoveTemp(Params)))
	{
		Result = Op->Result.NumRemoved;
	}

	return Result;
}

int32 UInventoryComponent::RemoveItemById(
	const FInventoryItemId& ItemId,
	int32 NumRemove,
	FGameplayTag GroupTag)
{
	int32 Result = NumRemove;
	AInventoryBase* Inventory = GetInventory();
	if (!IsValid(Inventory))
	{
		ITEMIZATION_ERROR_CONTEXT("Cannot remove item [%s] from inventory [%s] for %s. Inventory is invalid.",
			*ItemId.ToString(), *GetNameSafe(Inventory), *GetNameSafe(GetOwner()));
		return Result;
	}

	// Build our remove item operation parameters
	FInventoryOp_RemoveItem::FParams Params;
	Params.ItemId = ItemId;
	Params.NumRemove = NumRemove;
	Params.GroupTag = GroupTag;

	// Actually remove the item
	if (const TInventoryOpPtr<FInventoryOp_RemoveItem> Op = Inventory->RemoveItem(MoveTemp(Params)))
	{
		Result = Op->Result.NumRemoved;
	}

	return Result;
}

int32 UInventoryComponent::RemoveItem(
	TScriptInterface<IInventoryItemInstanceInterface> ItemInstance,
	int32 NumRemove,
	FGameplayTag GroupTag)
{
	int32 Result = NumRemove;
	AInventoryBase* Inventory = GetInventory();
	if (!IsValid(Inventory))
	{
		ITEMIZATION_ERROR_CONTEXT("Cannot remove item [%s] from inventory [%s] for %s. Inventory is invalid.",
			*GetNameSafe(ItemInstance.GetObject()), *GetNameSafe(Inventory), *GetNameSafe(GetOwner()));
		return Result;
	}

	// Build our remove item operation parameters
	FInventoryOp_RemoveItem::FParams Params;
	Params.ItemInstance = ItemInstance.GetObject();
	Params.NumRemove = NumRemove;
	Params.GroupTag = GroupTag;

	// Actually remove the item
	if (const TInventoryOpPtr<FInventoryOp_RemoveItem> Op = Inventory->RemoveItem(MoveTemp(Params)))
	{
		Result = Op->Result.NumRemoved;
	}

	return Result;
}

void UInventoryComponent::CreateInventory()
{
	UWorld* const World = GetWorld();

	const ENetMode NetMode = World->GetNetMode();
	check(NetMode != NM_Client);

	FActorSpawnParameters SpawnInfo;
	SpawnInfo.Owner = GetOwner();
	SpawnInfo.Instigator = GetOwner<APawn>();
	SpawnInfo.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	SpawnInfo.ObjectFlags |= RF_Transient;

	UClass* Class = InventoryClass;
	if (!IsValid(Class))
	{
		// Fall back to the default inventory class
		Class = AInventoryBase::StaticClass();
	}

	// Spawn the inventory
	AInventoryBase* SpawnedInventory =
		World->SpawnActor<AInventoryBase>(Class, SpawnInfo);

	MARK_PROPERTY_DIRTY_FROM_NAME(ThisClass, InventoryHandle, this);

	InventoryHandle.AssignInventory(SpawnedInventory);
	AuthorityInventory = SpawnedInventory;

	OnInventoryCreated(SpawnedInventory);
}

void UInventoryComponent::OnInventoryCreated(AInventoryBase* Inventory)
{
	Inventory->InventoryHandle = InventoryHandle;

	ITEMIZATION_DISPLAY_NET("Inventory [%s] created for %s.",
		*GetNameSafe(Inventory), *GetNameSafe(GetOwner()));

	if (bAttachInventoryToOwner)
	{
		AActor* OwnerActor = GetOwner();
		if (IsValid(OwnerActor))
		{
			// Only attach if not already attached.
			if (OwnerActor->GetRootComponent() && Inventory->GetRootComponent()->GetAttachParent() != OwnerActor->GetRootComponent())
			{
				Inventory->GetRootComponent()->DetachFromComponent(FDetachmentTransformRules::KeepRelativeTransform);
				Inventory->GetRootComponent()->SetRelativeLocationAndRotation(FVector::ZeroVector, FRotator::ZeroRotator);
				Inventory->AttachToComponent(OwnerActor->GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);
			}
		}
	}

	SetupInventory(Inventory);

	OnInventoryInitialized.Broadcast(Inventory);
	OnInventoryInitialized.Clear();
}


void UInventoryComponent::OnRep_InventoryHandle()
{
	if (InventoryHandle.IsValid())
	{
		OnInventoryCreated(InventoryHandle.GetInventory());
	}
	else
	{
		// If
	}
}
