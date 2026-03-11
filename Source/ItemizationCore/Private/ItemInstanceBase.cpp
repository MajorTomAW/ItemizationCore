// Author: Tom Werner (dc: majort), 2026


#include "ItemInstanceBase.h"

#include "InventoryBase.h"
#include "ItemizationCoreLogChannels.h"
#include "ItemizationCoreSettings.h"
#include "Iris/ReplicationSystem/ReplicationFragmentUtil.h"
#include "Items/Data/ItemComponentData_PickupBase.h"
#include "Net/UnrealNetwork.h"


#include UE_INLINE_GENERATED_CPP_BY_NAME(ItemInstanceBase)

UItemInstanceBase::UItemInstanceBase(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	auto ImplementedInBlueprint = [](const UFunction* Func) -> bool
	{
		return Func && ensure(Func->GetOuter())
			&& Func->GetOuter()->IsA(UBlueprintGeneratedClass::StaticClass());
	};

	bHasBlueprintOnAddedToInventory = ImplementedInBlueprint(GetClass()->FindFunctionByName(
		GET_FUNCTION_NAME_CHECKED(ThisClass, OnAddedToInventoryBP)));
}

void UItemInstanceBase::OnAddedToInventory(
	FInventoryItemEntry& OwningItemEntry,
	AInventoryBase* Inventory)
{
	if (HasAuthority())
	{
		OwningItemId = OwningItemEntry.GetItemId();
		ItemDefinition = OwningItemEntry.GetItemDefinition();
	}

	if (bHasBlueprintOnAddedToInventory)
	{
		OnAddedToInventoryBP(Inventory);
	}
}

void UItemInstanceBase::OnRemovedInventory(
	FInventoryItemEntry& OwningItemEntry,
	AInventoryBase* Inventory)
{
}

AInventoryBase* UItemInstanceBase::GetInventory() const
{
	if (HasAnyFlags(RF_ClassDefaultObject))
	{
		ITEMIZATION_LOG(Error, "GetInventory() cannot be called on a CDO of %s.", *GetName())
		return nullptr;
	}

	return GetTypedOuter<AInventoryBase>();
}

APlayerController* UItemInstanceBase::GetOwningPlayer() const
{
	return GetInventory()->GetOwner<APlayerController>();
}

int32 UItemInstanceBase::GetStackSize() const
{
	if (const auto* Entry = GetItemEntry())
	{
		return Entry->GetStackSize();
	}

	return 1;
}

float UItemInstanceBase::GetDurability() const
{
	if (HasDurability())
	{
		if (const auto* Entry = GetItemEntry())
		{
			return Entry->GetDurability();
		}
	}

	return -1.f;
}

bool UItemInstanceBase::HasDurability() const
{
	if (const UItemDefinitionBase* ItemDef = GetItemDefinition())
	{
		return ItemDef->HasTrait(UItemizationCoreSettings::Get()->HasDurabilityTag);
	}

	return false;
}

const FInventoryItemEntry* UItemInstanceBase::GetItemEntry() const
{
	return GetInventory()->FindItemEntryById(GetItemId());
}

bool UItemInstanceBase::HasAuthority() const
{
	if (const AInventoryBase* Inventory = GetInventory())
	{
		return Inventory->HasAuthority();
	}

	return false;
}

void UItemInstanceBase::RemoveFromInventory()
{
	AInventoryBase* Inventory = GetInventory();
	if (!ensure(Inventory))
	{
		return;
	}

	Inventory->RemoveItem(this);
}

bool UItemInstanceBase::CanBeDropped() const
{
	const UItemDefinitionBase* ItemDef = GetItemDefinition();
	if (!IsValid(ItemDef))
	{
		return false;
	}

	const auto* PickupData = ItemDef->GetItemData<FItemComponentData_PickupBase>();
	if (PickupData == nullptr)
	{
		return false;
	}

	return true;
}

AActor* UItemInstanceBase::DropItem()
{
	AInventoryBase* Inventory = GetInventory();
	if (!ensure(Inventory))
	{
		return nullptr;
	}

	if (!CanBeDropped())
	{
		return nullptr;
	}

	return Inventory->DropItem(this);
}

UWorld* UItemInstanceBase::GetWorld() const
{
	if (HasAnyFlags(RF_ClassDefaultObject))
	{
		// If we're CDO, we must return nullptr instead of calling Outer->GetWorld() to fool UObject::ImplementsGetWorld().
		ITEMIZATION_LOG(Error, "GetWorld() cannot be called on a CDO of %s", *GetName());
		return nullptr;
	}

	return GetOuter()->GetWorld();
}

int32 UItemInstanceBase::GetFunctionCallspace(UFunction* Function, FFrame* Stack)
{
	if (HasAnyFlags(RF_ClassDefaultObject) || !IsSupportedForNetworking())
	{
		// This handles absorbing authority/cosmetic.
		GEngine->GetGlobalFunctionCallspace(Function, this, Stack);
	}

	UObject* Outer = GetOuter();
	check(Outer);
	return Outer->GetFunctionCallspace(Function, Stack);
}

bool UItemInstanceBase::CallRemoteFunction(UFunction* Function, void* Parms, FOutParmRec* OutParms, FFrame* Stack)
{
	// No need to execute RPCs if we're on a pending kill
	if (!IsValid(this))
	{
		return false;
	}

	check(!HasAnyFlags(RF_ClassDefaultObject));
	check(GetOuter() != nullptr);

	AActor* Owner = CastChecked<AActor>(GetOuter());
	bool bProcessed = false;

	FWorldContext* const Context = GEngine->GetWorldContextFromWorld(GetWorld());
	if (Context != nullptr)
	{
		for (FNamedNetDriver& Driver : Context->ActiveNetDrivers)
		{
			if ((Driver.NetDriver != nullptr) &&
				(Driver.NetDriver->ShouldReplicateFunction(Owner, Function)))
			{
				Driver.NetDriver->ProcessRemoteFunction(Owner, Function, Parms, OutParms, Stack, this);
				bProcessed = true;
			}
		}
	}

	return bProcessed;
}

void UItemInstanceBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	UObject::GetLifetimeReplicatedProps(OutLifetimeProps);

	if (UBlueprintGeneratedClass* BPClass = Cast<UBlueprintGeneratedClass>(GetClass()))
	{
		BPClass->GetLifetimeBlueprintReplicationList(OutLifetimeProps);
	}

	DOREPLIFETIME(ThisClass, ItemDefinition)
	DOREPLIFETIME(ThisClass, OwningItemId)

	FDoRepLifetimeParams SharedParams;
	SharedParams.bIsPushBased = true;
}

void UItemInstanceBase::PostInitProperties()
{
	UObject::PostInitProperties();
}

#if UE_WITH_IRIS
void UItemInstanceBase::RegisterReplicationFragments(
	UE::Net::FFragmentRegistrationContext& Context,
	UE::Net::EFragmentRegistrationFlags RegistrationFlags)
{
	// Build description and allocate PropertyReplicationFragments for this UObject.
	UE::Net::FReplicationFragmentUtil::CreateAndRegisterFragmentsForObject(this, Context, RegistrationFlags);
}
#endif

#if WITH_EDITOR
EDataValidationResult UItemInstanceBase::IsDataValid(FDataValidationContext& Context) const
{
	return UObject::IsDataValid(Context);
}
#endif
