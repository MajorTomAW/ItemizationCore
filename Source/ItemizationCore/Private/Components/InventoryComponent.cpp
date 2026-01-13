// Author: Tom Werner (MajorT), 2025


#include "Components/InventoryComponent.h"

#include "ItemizationCoreLogChannels.h"
#include "Inventory/InventoryBase.h"
#include "Inventory/InventoryConfigAsset.h"
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
	TagContainer.AddTag(InventoryConfig->InventoryTag);
}

AInventoryBase* UInventoryComponent::GetInventory() const
{
	if (InventoryHandle.IsValid())
	{
		return InventoryHandle.GetInventory();
	}

	// Try to grab inventory from the owner
	if (const IInventoryOwnerInterface* InventoryAgent = Cast<IInventoryOwnerInterface>(GetOwner()))
	{
		return InventoryAgent->GetInventory();
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

TScriptInterface<IInventoryItemInstanceInterface> UInventoryComponent::FindItemInstanceByHandle(const FInventoryItemHandle& ItemHandle) const
{
	AInventoryBase* Inventory = GetInventory();
	if (!IsValid(Inventory))
	{
		return nullptr;
	}

	// Find the item instance in the inventory
	TScriptInterface<IInventoryItemInstanceInterface> ItemInstance = Inventory->FindItemInstanceByHandle(ItemHandle);
	if (IsValid(ItemInstance.GetObject()))
	{
		return ItemInstance;
	}

	ITEMIZATION_WARN_CONTEXT("Could not find item instance with handle [%s] in inventory [%s] for %s.",
		*ItemHandle.ToString(), *GetNameSafe(Inventory), *GetNameSafe(GetOwner()));

	return nullptr;
}

TArray<TScriptInterface<IInventoryItemInstanceInterface>> UInventoryComponent::GetInventoryItems() const
{
	AInventoryBase* Inventory = GetInventory();
	if (!IsValid(Inventory))
	{
		return TArray<TScriptInterface<IInventoryItemInstanceInterface>>();
	}

	return Inventory->GetAllItemInstancesAsInterfaces();
}

TArray<TScriptInterface<IInventoryItemInstanceInterface>> UInventoryComponent::GetInventoryItemsInGroup(FGameplayTag Group) const
{
	AInventoryBase* Inventory = GetInventory();
	if (!IsValid(Inventory))
	{
		return {};
	}

	return Inventory->GetItemInstancesInGroup(Group);
}

TArray<TScriptInterface<IInventoryItemInstanceInterface>> UInventoryComponent::GetInventoryItemsInGroups(TArray<FGameplayTag> Groups) const
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
}

FInventoryItemHandle UInventoryComponent::TryGiveItem(
	UItemDefinitionBase* ItemDefinition,
	int32 StackCount,
	UObject* SourceObject,
	FGameplayTag GroupTag,
	int32& OutNumCouldNotAdd)
{
	FInventoryItemHandle Result = FInventoryItemHandle::InvalidHandle;

	AInventoryBase* Inventory = GetInventory();
	if (!IsValid(Inventory))
	{
		ITEMIZATION_ERROR_CONTEXT("Cannot give item [%s] to inventory [%s] for %s. Inventory is invalid.",
			*GetNameSafe(ItemDefinition), *GetNameSafe(Inventory), *GetNameSafe(GetOwner()));
		return Result;
	}

	// Build our give item operation parameters
	FInventoryOp_GiveAction::FParams Params;
	Params.TargetInventory = Inventory;
	Params.NumGive = StackCount;
	Params.GroupTag = GroupTag;

	const AInventoryBase::FCreateItemEntryParams CreateItemParams { ItemDefinition, StackCount, SourceObject };

	// Actually give the item
	if (const TInventoryOpPtr<FInventoryOp_GiveAction> Op = Inventory->GiveItem(MoveTemp(Params), CreateItemParams))
	{
		Result = Op->Result.ItemHandle;

		// Let us know about how many items could not be added
		OutNumCouldNotAdd = Op->Result.Excess;
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

	if (HasAuthority())
	{
		InitInventoryGroups(Inventory);
	}

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
}

void UInventoryComponent::InitInventoryGroups(AInventoryBase* Inventory)
{
	if (!IsValid(InventoryConfig))
	{
		return;
	}

	Inventory->InitializeInventorySlots(InventoryConfig);
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
