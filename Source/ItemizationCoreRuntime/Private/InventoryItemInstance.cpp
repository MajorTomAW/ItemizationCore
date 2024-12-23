// Fill out your copyright notice in the Description page of Project Settings.


#include "InventoryItemInstance.h"

#if UE_WITH_IRIS
#include "Iris/ReplicationSystem/ReplicationFragmentUtil.h"
#endif

#include "InventoryItemEntry.h"
#include "ItemizationCoreLog.h"
#include "Net/UnrealNetwork.h"
#include "ActorComponents/InventoryManager.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(InventoryItemInstance)

#define LOCTEXT_NAMESPACE "ItemizationCore"

#define ENSURE_ITEM_IS_INSTANTIATED_OR_RETURN(FunctionName, ReturnValue) \
{ \
	if (!ensure(IsInstantiated())) \
	{ \
		ITEMIZATION_LOG(Error, TEXT("%s: " #FunctionName" cannot be called on a non-instanced item."), *GetPathName()); \
		return ReturnValue; \
	} \
} \

UInventoryItemInstance::UInventoryItemInstance(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	ScopeLockCount = 0;
	CurrentState = ECurrentItemState::None;
}

UWorld* UInventoryItemInstance::GetWorld() const
{
	if (!IsInstantiated())
	{
		// If we're CDO, we must return nullptr instead of calling Outer->GetWorld() to fool UObject::ImplementsGetWorld.
		return nullptr;
	}

	return GetOuter()->GetWorld();
}

int32 UInventoryItemInstance::GetFunctionCallspace(UFunction* Function, FFrame* Stack)
{
	if (HasAnyFlags(RF_ClassDefaultObject) || !IsSupportedForNetworking())
	{
		// This handles absorbing authority/cosmetic
		return GEngine->GetGlobalFunctionCallspace(Function, this, Stack);
	}

	check(GetOuter() != nullptr);
	return GetOuter()->GetFunctionCallspace(Function, Stack);
}

bool UInventoryItemInstance::CallRemoteFunction(UFunction* Function, void* Parms, FOutParmRec* OutParms, FFrame* Stack)
{
	check(!HasAnyFlags(RF_ClassDefaultObject));
	check(GetOuter() != nullptr);

	AActor* Owner = CastChecked<AActor>(GetOuter());
	bool bProcessed = false;

	FWorldContext* const Context = GEngine->GetWorldContextFromWorld(GetWorld());
	if (Context != nullptr)
	{
		for (FNamedNetDriver& Driver : Context->ActiveNetDrivers)
		{
			if (Driver.NetDriver != nullptr && Driver.NetDriver->ShouldReplicateFunction(Owner, Function))
			{
				Driver.NetDriver->ProcessRemoteFunction(Owner, Function, Parms, OutParms, Stack, this);
				bProcessed = true;
			}
		}
	}

	return bProcessed;
}

bool UInventoryItemInstance::IsSupportedForNetworking() const
{
	return true;
}

void UInventoryItemInstance::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	UObject::GetLifetimeReplicatedProps(OutLifetimeProps);

	if (UBlueprintGeneratedClass* BPClass = Cast<UBlueprintGeneratedClass>(GetClass()))
	{
		BPClass->GetLifetimeBlueprintReplicationList(OutLifetimeProps);
	}
}

#if UE_WITH_IRIS
void UInventoryItemInstance::RegisterReplicationFragments(UE::Net::FFragmentRegistrationContext& Context, UE::Net::EFragmentRegistrationFlags RegistrationFlags)
{
	// Build descriptors and allocate PropertyReplicationFragments for this object
	UE::Net::FReplicationFragmentUtil::CreateAndRegisterFragmentsForObject(this, Context, RegistrationFlags);
}
#endif

bool UInventoryItemInstance::IsInstantiated() const
{
	return !HasAllFlags(RF_ClassDefaultObject);
}

void UInventoryItemInstance::PostNetInit()
{
	// Here we're dynamically spawned from replication.
	// Which means we need to init out properties ourselves.
	if (CurrentInventoryManager == nullptr)
	{
		AActor* OwnerActor = GetTypedOuter<AActor>();
		if (ensure(OwnerActor))
		{
			UInventoryManager* InventoryManager = OwnerActor->FindComponentByClass<UInventoryManager>(); //@TODO: Abstract this to a function.
			if (ensure(InventoryManager))
			{
				CurrentInventoryManager = InventoryManager;
			}
		}
	}
}

void UInventoryItemInstance::SetCurrentEntryInfo(UInventoryManager* InventoryManager, const FInventoryItemEntryHandle& InHandle)
{
	if (IsInstantiated())
	{
		CurrentEntryHandle = InHandle;
		CurrentInventoryManager = InventoryManager;
	}
}

void UInventoryItemInstance::OnAddedToInventory(UInventoryManager* InventoryManager, const FInventoryItemEntry& ItemEntry)
{
	SetCurrentEntryInfo(InventoryManager, ItemEntry.Handle);

	//@TODO: Get the avatar from the inventory manager
}

void UInventoryItemInstance::OnRemovedFromInventory(UInventoryManager* InventoryManager, const FInventoryItemEntry& ItemEntry)
{
	// Nothing to do here. Can be overridden by subclasses
}

void UInventoryItemInstance::OnAvatarSet(AActor* Avatar, const FInventoryItemEntry& ItemEntry)
{
	// Nothing to do here. Can be overridden by subclasses
}

FInventoryItemEntryHandle UInventoryItemInstance::GetCurrentItemHandle() const
{
	ENSURE_ITEM_IS_INSTANTIATED_OR_RETURN(GetCurrentItemHandle, FInventoryItemEntryHandle());
	return CurrentEntryHandle;
}

FInventoryItemEntry* UInventoryItemInstance::GetCurrentItemEntry() const
{
	ENSURE_ITEM_IS_INSTANTIATED_OR_RETURN(GetCurrentItemEntry, nullptr);
	check(CurrentInventoryManager.IsValid());

	UInventoryManager* const InventoryManager = CurrentInventoryManager.Get();
	return InventoryManager->FindItemEntryFromHandle(CurrentEntryHandle);
}

#undef LOCTEXT_NAMESPACE
