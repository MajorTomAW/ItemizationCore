// Author: Tom Werner (dc: majort), 2026


#include "Components/InventoryComponent.h"

#include "InventoryBase.h"
#include "ItemizationCoreLogChannels.h"
#include "Config/InventoryConfig.h"
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
	bAttachInventoryToOwner = false;

	InventoryClass = AInventoryBase::StaticClass();
}

AInventoryBase* UInventoryComponent::GetInventory_Implementation() const
{
	return Inventory;
}

void UInventoryComponent::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	FDoRepLifetimeParams SharedParams;
	SharedParams.bIsPushBased = true;

	DOREPLIFETIME_WITH_PARAMS_FAST(ThisClass, Inventory, SharedParams);
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
		if (IsValid(Inventory))
		{
			ITEMIZATION_LOG(Warning, "Cleaning up old inventory [%s] for %s.",
				*GetNameSafe(Inventory), *GetNameSafe(GetOwner()));

			Inventory = nullptr;
		}

		ITEMIZATION_LOG(Verbose, "Acquiring inventory on initialize for %s.",
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
	if (AInventoryBase* MyInventory = Execute_GetInventory(this))
	{
		Delegate.ExecuteIfBound(MyInventory);
	}
	else
	{
		OnInventoryInitialized.Add(MoveTemp(Delegate));
	}
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
	MARK_PROPERTY_DIRTY_FROM_NAME(ThisClass, Inventory, this);
	Inventory = World->SpawnActor<AInventoryBase>(Class, SpawnInfo);

	OnInventoryCreated(Inventory);
}

void UInventoryComponent::OnInventoryCreated(AInventoryBase* MyInventory)
{
	//Inventory->InventoryHandle = InventoryHandle;

	ITEMIZATION_LOG(Log, "Inventory [%s] created for %s.",
		*GetNameSafe(MyInventory), *GetNameSafe(GetOwner()));

	if (bAttachInventoryToOwner)
	{
		AActor* OwnerActor = GetOwner();
		if (IsValid(OwnerActor))
		{
			// Only attach if not already attached.
			if (OwnerActor->GetRootComponent() && MyInventory->GetRootComponent()->GetAttachParent() != OwnerActor->GetRootComponent())
			{
				MyInventory->GetRootComponent()->DetachFromComponent(FDetachmentTransformRules::KeepRelativeTransform);
				MyInventory->GetRootComponent()->SetRelativeLocationAndRotation(FVector::ZeroVector, FRotator::ZeroRotator);
				MyInventory->AttachToComponent(OwnerActor->GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);
			}
		}
	}

	SetupInventory(MyInventory);

	OnInventoryInitialized.Broadcast(MyInventory);
	OnInventoryInitialized.Clear();
}

void UInventoryComponent::SetupInventory(AInventoryBase* MyInventory)
{
	if (HasAuthority())
	{
		InitInventoryGroups(MyInventory);
	}
}

void UInventoryComponent::InitInventoryGroups(AInventoryBase* MyInventory)
{
	if (!IsValid(InventoryConfig))
	{
		ITEMIZATION_LOG(Warning, "Unable to init inventory groups for %s with an invalid inventory config.",
			*GetNameSafe(GetOwner()))
		return;
	}

	MyInventory->InitializeInventorySlots(InventoryConfig);
}

void UInventoryComponent::OnRep_Inventory()
{
	if (IsValid(Inventory))
	{
		OnInventoryCreated(Inventory);
	}
}
