// Copyright © 2025 MajorT. All Rights Reserved.


#include "Inventory/SlottableInventory.h"

#include "Inventory/InventoryProperties.h"
#include "Net/UnrealNetwork.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(SlottableInventory)

ASlottableInventory::ASlottableInventory(const FObjectInitializer& ObjectInitializer)
	: AInventoryBase(ObjectInitializer.DoNotCreateDefaultSubobject(TEXT("Sprite")))
{
#if WITH_EDITORONLY_DATA
	bHiddenEd = true;
#endif

	SetNetUpdateFrequency(1.0f);

	bReplicateSlotsToClients = true;

	bNetUseOwnerRelevancy = true;
	bOnlyRelevantToOwner = !bReplicateSlotsToClients;
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = false;
}

void ASlottableInventory::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	FDoRepLifetimeParams SharedParams;
	SharedParams.bIsPushBased = true;
	SharedParams.Condition = bReplicateSlotsToClients ? COND_None : COND_ReplayOrOwner;

	DOREPLIFETIME_WITH_PARAMS_FAST(ThisClass, InventorySlots, SharedParams);
	
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
}

void ASlottableInventory::PostInitInventory()
{
	// Make sure to only init this inventory on authority
	if (!HasAuthority() || !GetWorld()->IsGameWorld())
	{
		return;
	}
	
	const FSlottableInventoryProperties* Props = GetInventoryProperties<FSlottableInventoryProperties>();
	check(Props);

	int32 DesiredNumSlots = 0;
	if (ensure(ParentInventory))
	{
		DesiredNumSlots = FMath::Max(0, ParentInventory->GetInventoryProperties<FInventoryPropertiesBase>()->TotalSlotsOverride);

		// Iterate over all children and add their slots to the total
		for (const AInventoryBase* Child : ParentInventory->ChildInventoryList)
		{
			if (Child == nullptr)
			{
				continue;
			}

			if (const FInventoryPropertiesBase* ChildProps = Child->GetInventoryProperties<FInventoryPropertiesBase>())
			{
				DesiredNumSlots += FMath::Max(0, ChildProps->TotalSlotsOverride);
			}
		}
	}
	// Subtract our own slots from the total
	DesiredNumSlots -= FMath::Max(0, Props->TotalSlotsOverride);

	// Make sure we have at least the minimum number of slots
	if (DesiredNumSlots < MinSlots)
	{
		DesiredNumSlots = MinSlots;
	}

	// In case we exceed the max slots, we need to clamp it
	if ((Props->TotalSlotsOverride > 0) &&
		(DesiredNumSlots > Props->TotalSlotsOverride))
	{
		DesiredNumSlots = FMath::Min(DesiredNumSlots, Props->TotalSlotsOverride);
	}

	// Pre-allocate the slots
	InventorySlots.Slots.Reserve(DesiredNumSlots);
	for (int32 i = 0; i < DesiredNumSlots; ++i)
	{
		FInventorySlotEntry& NewSlot = InventorySlots.Slots.Add_GetRef(FInventorySlotEntry(i));
		NewSlot.SetEnabled(true);

		InventorySlots.MarkItemDirty(NewSlot);
	}
	ForceNetUpdate();
}


void ASlottableInventory::OnRep_InventorySlots()
{
}
