// Author: Tom Werner (MajorT), 2025


#include "Components/InventoryComponent.h"

#include "ItemizationLogChannels.h"
#include "Inventory/InventoryBase.h"

#include "Net/UnrealNetwork.h"

#if WITH_EDITOR
#include "Misc/DataValidation.h"
#endif

#include UE_INLINE_GENERATED_CPP_BY_NAME(InventoryComponent)

#define LOCTEXT_NAMESPACE "InventoryComponent"

UInventoryComponent::UInventoryComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryComponentTick.bStartWithTickEnabled = false;
	PrimaryComponentTick.bCanEverTick = true;
	
	SetIsReplicatedByDefault(true);
	bReplicateUsingRegisteredSubObjectList = true;

	bWantsInitializeComponent = true;
	bShouldAcquireInventoryOnInitialize = true;
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

void UInventoryComponent::GetOwnedGameplayTags(FGameplayTagContainer& TagContainer) const
{
	TagContainer.AddTag(InventoryConfig->InventoryTag);
}

void UInventoryComponent::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
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

	// Only continue if we're a server
	if ((GetNetMode() != NM_Client) &&
		IsValid(this) &&
		bShouldAcquireInventoryOnInitialize)
	{
		// Make sure we're starting with a clean inventory
		if (InventoryHandle.IsValid())
		{
			ITEMIZATION_S_WARN("Cleaning up old inventory handle [%s] for %s.",
				*GetNameSafe(InventoryHandle.GetInventory()), *GetNameSafe(GetOwner()));
			InventoryHandle.Reset();
		}
		
		ITEMIZATION_S_VERBOSE("Acquiring inventory on initialize for %s.",
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
EDataValidationResult UInventoryComponent::IsDataValid(class FDataValidationContext& Context) const
{
	EDataValidationResult Result = Super::IsDataValid(Context);

	return Result;
}
#endif

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

void UInventoryComponent::OnRep_InventoryHandle()
{
	if (InventoryHandle.IsValid())
	{
		OnInventoryCreated(InventoryHandle.GetInventory());
	}
}


void UInventoryComponent::OnInventoryCreated(AInventoryBase* Inventory)
{
	Inventory->InventoryHandle = InventoryHandle;
	
	if (HasAuthority())
	{
		InitInventoryGroups();
	}

	ITEMIZATION_N_DISPLAY("Inventory [%s] created for %s.",
		*GetNameSafe(Inventory), *GetNameSafe(GetOwner()));
}

void UInventoryComponent::InitInventoryGroups()
{
	for (const auto& Config : InventoryConfig->InventoryGroupConfigs)
	{
		// Should always be a valid group type
		if (!ensure(Config.GroupType.IsValid()))
		{
			continue;
		}
	}
}

#undef LOCTEXT_NAMESPACE