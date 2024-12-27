// Fill out your copyright notice in the Description page of Project Settings.


#include "InventoryItemInstance.h"

#if UE_WITH_IRIS
#include "Iris/ReplicationSystem/ReplicationFragmentUtil.h"
#endif

#include "InventoryItemEntry.h"
#include "ItemizationCoreLog.h"
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
	auto ImplementedInBlueprint = [](const UFunction* Func)->bool
	{
		return Func && ensure(Func->GetOuter()) &&
			Func->GetOuter()->IsA(UBlueprintGeneratedClass::StaticClass());
	};

	{
		static FName FuncName = FName(TEXT("K2_OnAddedToInventory"));
		UFunction* OnAddedToInventoryFunction = GetClass()->FindFunctionByName(FuncName);
		bHasBlueprintAddedToInventory = ImplementedInBlueprint(OnAddedToInventoryFunction);
	}

	{
		static FName FuncName = FName(TEXT("K2_OnRemovedFromInventory"));
		UFunction* OnRemovedFromInventoryFunction = GetClass()->FindFunctionByName(FuncName);
		bHasBlueprintRemovedFromInventory = ImplementedInBlueprint(OnRemovedFromInventoryFunction);
	}
	
	ScopeLockCount = 0;
	CurrentState = ECurrentItemState::None;
	CurrentInventoryData = nullptr;
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
	if (CurrentInventoryData == nullptr)
	{
		AActor* OwnerActor = GetTypedOuter<AActor>();
		if (ensure(OwnerActor))
		{
			UInventoryManager* InventoryManager = OwnerActor->FindComponentByClass<UInventoryManager>(); //@TODO: Abstract this to a function.
			if (ensure(InventoryManager))
			{
				CurrentInventoryData = InventoryManager->InventoryData.Get();
			}
		}
	}
}

void UInventoryItemInstance::SetCurrentEntryInfo(const FInventoryItemEntryHandle InHandle, const FItemizationCoreInventoryData* InventoryData)
{
	if (IsInstantiated())
	{
		CurrentEntryHandle = InHandle;
		CurrentInventoryData = InventoryData;
	}
}

void UInventoryItemInstance::OnAddedToInventory(const FInventoryItemEntry& ItemEntry, const FItemizationCoreInventoryData* InventoryData)
{
	SetCurrentEntryInfo(ItemEntry.Handle, InventoryData);

	for (const FItemComponentData* Component : ItemEntry.Definition->GetItemComponents())
	{
		Component->OnItemInstanceCreated(ItemEntry, InventoryData);
	}

	if (InventoryData && InventoryData->AvatarActor.IsValid())
	{
		OnAvatarSet(ItemEntry, InventoryData);
	}

	if (bHasBlueprintAddedToInventory)
	{
		K2_OnAddedToInventory();
	}
}

void UInventoryItemInstance::OnRemovedFromInventory(const FInventoryItemEntry& ItemEntry, const FItemizationCoreInventoryData* InventoryData)
{
	for (const FItemComponentData* Component : ItemEntry.Definition->GetItemComponents())
	{
		Component->OnItemInstanceDestroyed(ItemEntry, InventoryData);
	}

	if (bHasBlueprintRemovedFromInventory)
	{
		K2_OnRemovedFromInventory();
	}
	
	// Nothing else to do here. Can be overridden by subclasses
}

void UInventoryItemInstance::OnAvatarSet(const FInventoryItemEntry& ItemEntry, const FItemizationCoreInventoryData* InventoryData)
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
	check(CurrentInventoryData);

	UInventoryManager* const InventoryManager = GetOwningInventoryManager_Checked();
	return InventoryManager->FindItemEntryFromHandle(CurrentEntryHandle);
}

UItemDefinition* UInventoryItemInstance::GetCurrentItemDefinition() const
{
	return GetCurrentItemEntry()->Definition;
}

const FItemizationCoreInventoryData* UInventoryItemInstance::GetCurrentInventoryData() const
{
	ENSURE_ITEM_IS_INSTANTIATED_OR_RETURN(GetCurrentInventoryData, nullptr);
	return CurrentInventoryData;
}

UInventoryManager* UInventoryItemInstance::GetOwningInventoryManager() const
{
	if (!ensure(CurrentInventoryData))
	{
		return nullptr;
	}

	return CurrentInventoryData->InventoryManager.Get();
}

UInventoryManager* UInventoryItemInstance::GetOwningInventoryManager_Checked() const
{
	UInventoryManager* InventoryManager = CurrentInventoryData ? CurrentInventoryData->InventoryManager.Get() : nullptr;
	check(InventoryManager);

	return InventoryManager;
}

UInventoryManager* UInventoryItemInstance::GetOwningInventoryManager_Ensured() const
{
	UInventoryManager* InventoryManager = CurrentInventoryData ? CurrentInventoryData->InventoryManager.Get() : nullptr;
	ensure(InventoryManager);

	return InventoryManager;
}

EUserFacingItemState UInventoryItemInstance::GetUserFacingState() const
{
	const ECurrentItemState State = GetCurrentState();
	
	if (State == ECurrentItemState::Active)
	{
		return EUserFacingItemState::Equipped;
	}
	
	if (State == ECurrentItemState::Activating || State == ECurrentItemState::Active)
	{
		return EUserFacingItemState::EquippedAndActive;
	}

	return EUserFacingItemState::Owned;
}

bool UInventoryItemInstance::HasAuthority() const
{
	if (!IsInstantiated())
	{
		return false;
	}

	if (CurrentInventoryData == nullptr)
	{
		return false;
	}

	return CurrentInventoryData->HasNetAuthority();
}

bool UInventoryItemInstance::IsLocallyControlled() const
{
	const FItemizationCoreInventoryData* const DataPtr = GetCurrentInventoryData();
	if (DataPtr->OwnerActor.IsValid())
	{
		return DataPtr->IsLocallyControlled();
	}

	return false;
}

FItemizationCoreInventoryData UInventoryItemInstance::GetInventoryData() const
{
	if (!ensure(CurrentInventoryData))
	{
		return FItemizationCoreInventoryData();
	}
	return *CurrentInventoryData;
}

AActor* UInventoryItemInstance::GetOwningActorFromInventoryData() const
{
	ENSURE_ITEM_IS_INSTANTIATED_OR_RETURN(GetOwningActorFromInventoryData, nullptr);

	if (!ensure(CurrentInventoryData))
	{
		return nullptr;
	}

	return CurrentInventoryData->OwnerActor.Get();
}

AActor* UInventoryItemInstance::GetAvatarActorFromInventoryData() const
{
	ENSURE_ITEM_IS_INSTANTIATED_OR_RETURN(GetAvatarActorFromInventoryData, nullptr);

	if (!ensure(CurrentInventoryData))
	{
		return nullptr;
	}

	return CurrentInventoryData->AvatarActor.Get();
}

UObject* UInventoryItemInstance::GetSourceObject() const
{
	FInventoryItemEntry* Entry = GetCurrentItemEntry();
	if (Entry)
	{
		return Entry->SourceObject.Get();
	}

	return nullptr;
}

void UInventoryItemInstance::IncrementListLock() const
{
	++ScopeLockCount;
}

void UInventoryItemInstance::DecrementListLock() const
{
	if (--ScopeLockCount == 0)
	{
		
	}
}

#undef LOCTEXT_NAMESPACE
