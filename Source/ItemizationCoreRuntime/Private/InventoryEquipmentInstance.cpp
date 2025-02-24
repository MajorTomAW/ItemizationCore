// Copyright Epic Games, Inc. All Rights Reserved.


#include "InventoryEquipmentInstance.h"

#if UE_WITH_IRIS
#include "Iris/ReplicationSystem/ReplicationFragmentUtil.h"
#endif

#include "InventoryEquipmentEntry.h"
#include "InventoryItemInstance.h"
#include "ItemizationCoreLog.h"
#include "ActorComponents/EquipmentManager.h"
#include "ActorComponents/InventoryManager.h"
#include "Engine/AssetManager.h"
#include "Engine/StreamableManager.h"
#include "Engine/Engine.h"
#include "Engine/BlueprintGeneratedClass.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(InventoryEquipmentInstance)

using namespace Itemization;

#define ENSURE_EQUIPMENT_IS_INSTANTIATED_OR_RETURN(FunctionName, ReturnValue) \
{ \
	if (!ensure(IsInstantiated())) \
	{ \
		ITEMIZATION_LOG(Error, TEXT("%s: " #FunctionName" cannot be called on a non-instanced equipment."), *GetPathName()); \
		return ReturnValue; \
	} \
} \

UInventoryEquipmentInstance::UInventoryEquipmentInstance(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	auto ImplementedInBlueprint = [](const UFunction* Func)->bool
	{
		return Func && ensure(Func->GetOuter()) &&
			Func->GetOuter()->IsA(UBlueprintGeneratedClass::StaticClass());
	};

	{
		static FName FuncName = FName(TEXT("K2_OnEquipped"));
		UFunction* OnEquippedFunction = GetClass()->FindFunctionByName(FuncName);
		bHasBlueprintEquipped = ImplementedInBlueprint(OnEquippedFunction);
	}

	{
		static FName FuncName = FName(TEXT("K2_OnUnequipped"));
		UFunction* OnUnequippedFunction = GetClass()->FindFunctionByName(FuncName);
		bHasBlueprintUnequipped = ImplementedInBlueprint(OnUnequippedFunction);
	}
	
	CurrentInventoryData = nullptr;
}

UWorld* UInventoryEquipmentInstance::GetWorld() const
{
	if (!IsInstantiated())
	{
		// If we're CDO, we must return nullptr instead of calling Outer->GetWorld() to fool UObject::ImplementsGetWorld.
		return nullptr;
	}
	
	return GetOuter()->GetWorld();
}

int32 UInventoryEquipmentInstance::GetFunctionCallspace(UFunction* Function, FFrame* Stack)
{
	if (HasAnyFlags(RF_ClassDefaultObject) || !IsSupportedForNetworking())
	{
		// This handles absorbing authority/cosmetic
		return GEngine->GetGlobalFunctionCallspace(Function, this, Stack);
	}

	check(GetOuter() != nullptr);
	return GetOuter()->GetFunctionCallspace(Function, Stack);
}

bool UInventoryEquipmentInstance::CallRemoteFunction(UFunction* Function, void* Parms, FOutParmRec* OutParms, FFrame* Stack)
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

bool UInventoryEquipmentInstance::IsSupportedForNetworking() const
{
	return true;
}

void UInventoryEquipmentInstance::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	UObject::GetLifetimeReplicatedProps(OutLifetimeProps);

	if (UBlueprintGeneratedClass* BPClass = Cast<UBlueprintGeneratedClass>(GetClass()))
	{
		BPClass->GetLifetimeBlueprintReplicationList(OutLifetimeProps);
	}
}

#if UE_WITH_IRIS
void UInventoryEquipmentInstance::RegisterReplicationFragments(UE::Net::FFragmentRegistrationContext& Context, UE::Net::EFragmentRegistrationFlags RegistrationFlags)
{
	// Build descriptors and allocate PropertyReplicationFragments for this object
	UE::Net::FReplicationFragmentUtil::CreateAndRegisterFragmentsForObject(this, Context, RegistrationFlags);
}
#endif

bool UInventoryEquipmentInstance::IsInstantiated() const
{
	return !HasAllFlags(RF_ClassDefaultObject);
}

bool UInventoryEquipmentInstance::HasAnyEquipmentData() const
{
	return true; // Always true for now
}

void UInventoryEquipmentInstance::SetCurrentEntryInfo(const FInventoryItemEntryHandle InHandle, const FItemizationCoreInventoryData* InventoryData)
{
	if (IsInstantiated())
	{
		CurrentEntryHandle = InHandle;
		CurrentInventoryData = InventoryData;
	}
}

void UInventoryEquipmentInstance::OnEquipped(const FInventoryEquipmentEntry& EquipmentEntry, const FItemizationCoreInventoryData* InventoryData)
{
	SetCurrentEntryInfo(EquipmentEntry.Handle, InventoryData);
	
	const UItemDefinition* ItemDefinition = EquipmentEntry.Definition.Get();

	if (ItemDefinition == nullptr)
	{
		ITEMIZATION_Net_LOG(Warning, this, TEXT("OnEquipped: Item definition is null for equipment entry %s."), *EquipmentEntry.Handle.ToString());
		return;
	}

	for (const FItemComponentData* Component : ItemDefinition->GetItemComponents())
	{
		Component->OnItemStateChanged(EquipmentEntry.Handle, EUserFacingItemState::Equipped);
	}

	if (InventoryData && InventoryData->AvatarActor.IsValid())
	{
		OnAvatarSet(EquipmentEntry, InventoryData);

		ITEMIZATION_Net_LOG(Display, this, TEXT("Equipped Item [%s] '%s'"), *EquipmentEntry.Handle.ToString(), *ItemDefinition->GetPathName());
	}
	else
	{
		if (InventoryData)
		{
			ITEMIZATION_Net_LOG(Warning, this, TEXT("OnEquipped: Avatar actor is null for equipment entry %s."),
				*EquipmentEntry.Handle.ToString());
		}
		else
		{
			ITEMIZATION_Net_LOG(Warning, this, TEXT("OnEquipped: Inventory data is null for equipment entry %s."),
				*EquipmentEntry.Handle.ToString());
		}
	}
}

void UInventoryEquipmentInstance::OnUnequipped(const FInventoryEquipmentEntry& EquipmentEntry, const FItemizationCoreInventoryData* InventoryData)
{
	for (const FItemComponentData* Component : EquipmentEntry.Definition->GetItemComponents())
	{
		Component->OnItemStateChanged(EquipmentEntry.Handle, EUserFacingItemState::Owned);
	}

	if (bHasBlueprintUnequipped)
	{
		K2_OnUnequipped(InventoryData->AvatarActor.Get());
	}

	DestroyEquipmentActors();
}

FInventoryItemEntryHandle UInventoryEquipmentInstance::GetCurrentEntryHandle() const
{
	ENSURE_EQUIPMENT_IS_INSTANTIATED_OR_RETURN(GetCurrentEntryHandle, FInventoryItemEntryHandle::NullHandle);
	return CurrentEntryHandle;
}

FInventoryEquipmentEntry* UInventoryEquipmentInstance::GetCurrentEquipmentEntry() const
{
	ENSURE_EQUIPMENT_IS_INSTANTIATED_OR_RETURN(GetCurrentEquipmentEntry, nullptr);
	check(CurrentInventoryData);

	UEquipmentManager* const EquipmentManager = GetOwningEquipmentManager_Checked();
	return EquipmentManager->FindEquipmentEntryFromHandle(CurrentEntryHandle);
}

UItemDefinition* UInventoryEquipmentInstance::GetItemDefinition() const
{
	return GetCurrentEquipmentEntry()->Definition;
}

const FItemizationCoreInventoryData* UInventoryEquipmentInstance::GetCurrentInventoryData() const
{
	ENSURE_EQUIPMENT_IS_INSTANTIATED_OR_RETURN(GetCurrentInventoryData, nullptr);
	return CurrentInventoryData;
}

bool UInventoryEquipmentInstance::HasAuthority() const
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

UEquipmentManager* UInventoryEquipmentInstance::GetOwningEquipmentManager() const
{
	if (!ensure(CurrentInventoryData))
	{
		return nullptr;
	}

	return UEquipmentManager::FindEquipmentManager(CurrentInventoryData->AvatarActor.Get());
}

UEquipmentManager* UInventoryEquipmentInstance::GetOwningEquipmentManager_Checked() const
{
	UEquipmentManager* EquipmentManager = GetOwningEquipmentManager();
	check(EquipmentManager);
	return EquipmentManager;
}

UEquipmentManager* UInventoryEquipmentInstance::GetOwningEquipmentManager_Ensured() const
{
	UEquipmentManager* EquipmentManager = GetOwningEquipmentManager();
	ensure(EquipmentManager);
	return EquipmentManager;
}

void UInventoryEquipmentInstance::OnAvatarSet(const FInventoryEquipmentEntry& EquipmentEntry, const FItemizationCoreInventoryData* InventoryData)
{
	if (bHasBlueprintEquipped)
	{
		K2_OnEquipped(InventoryData->AvatarActor.Get());
	}

	if (HasAnyEquipmentData())
	{
		OnSpawnEquipmentActors(EquipmentEntry, InventoryData);
	}
}

void UInventoryEquipmentInstance::OnSpawnEquipmentActors(
	const FInventoryEquipmentEntry& EquipmentEntry, const FItemizationCoreInventoryData* InventoryData)
{
	TArray<FItemizationEquipmentSpawnQuery> Queries;
	const UItemDefinition* ItemDefinition = EquipmentEntry.Definition;
	ItemDefinition->EquipmentData.Get<FEquipmentComponentData>().QuerySpawnParams(Queries, GetPawn());

	TArray<FSoftObjectPath> AssetsToStream;
	FStreamableManager& Streamable = UAssetManager::GetStreamableManager();

	for (const auto& Query : Queries)
	{
		if (!Query.IsValid())
		{
			continue;
		}

		AssetsToStream.AddUnique(Query.ActorToSpawn.ToSoftObjectPath());
	}

	// Request the async load of the equipment actors.
	Streamable.RequestAsyncLoad(AssetsToStream, FStreamableDelegate::CreateUObject(this, &ThisClass::SpawnEquipmentActorsDeferred, Queries));
}

void UInventoryEquipmentInstance::SpawnEquipmentActorsDeferred(TArray<FItemizationEquipmentSpawnQuery> Queries)
{
	for (const auto& Query : Queries)
	{
		AActor* NewActor = GetWorld()->SpawnActorDeferred<AActor>(
			Query.ActorToSpawn.Get(), FTransform::Identity, GetPawn(), GetPawn());
		check(NewActor);
		
		NewActor->FinishSpawning(FTransform::Identity, true);
		NewActor->SetActorRelativeTransform(Query.RelativeTransform);
		NewActor->AttachToComponent(Query.AttachTarget.Get(), FAttachmentTransformRules::KeepRelativeTransform, Query.SocketName);

		SpawnedEquipmentActors.AddUnique(NewActor);
	}

	PostSpawnEquipmentActors();
}

void UInventoryEquipmentInstance::DestroyEquipmentActors()
{
	for (AActor* Actor : SpawnedEquipmentActors)
	{
		if (!IsValid(Actor))
		{
			return;
		}

		Actor->Destroy();
	}
}

UObject* UInventoryEquipmentInstance::GetInstigatorTyped(TSubclassOf<UInventoryItemInstance> Type) const
{
	UObject* Instigator = nullptr;
	const FInventoryEquipmentEntry* EquipmentEntry = GetCurrentEquipmentEntry();

	if (EquipmentEntry && EquipmentEntry->SourceObject && EquipmentEntry->SourceObject->IsA(Type))
	{
		return EquipmentEntry->SourceObject.Get();
	}

	return nullptr;
}

UObject* UInventoryEquipmentInstance::GetInstigator() const
{
	return GetInstigatorTyped(UInventoryItemInstance::StaticClass());
}

FItemizationCoreInventoryData UInventoryEquipmentInstance::GetInventoryData() const
{
	if (!ensure(CurrentInventoryData))
	{
		return FItemizationCoreInventoryData();
	}

	return *CurrentInventoryData;
}

const FItemizationCoreInventoryData* UInventoryEquipmentInstance::GetInventoryDataPtr() const
{
	return CurrentInventoryData;
}

UItemDefinition* UInventoryEquipmentInstance::K2_GetCurrentItemDefinition_Typed(TSubclassOf<UItemDefinition> Type) const
{
	UItemDefinition* ItemDef = GetItemDefinition();

	if (ItemDef->GetClass()->IsChildOf(Type))
	{
		return ItemDef;
	}

	return nullptr;
}
