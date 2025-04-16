// Copyright © 2025 MajorT. All Rights Reserved.


#include "Inventory/SlottableInventory.h"

#include "ItemizationLogChannels.h"
#include "Inventory/Inventory.h"
#include "Inventory/InventoryProperties.h"
#include "Inventory/Messaging/InventoryChangeMessage.h"
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

	// Add the initial slots
	AddInitialSlots();

	// Setup delegates
	if (AInventory* Parent = CastChecked<AInventory>(ParentInventory))
	{
		BindToInventoryDelegates(Parent);
	}
	
	ForceNetUpdate();
}

void ASlottableInventory::AddInitialSlots()
{
	const FSlottableInventoryProperties* Props = GetInventoryProperties<FSlottableInventoryProperties>();
	check(Props);

	int32 DesiredNumSlots = 0;
	if (ensure(ParentInventory))
	{
		DesiredNumSlots = ParentInventory->GetInventoryProperties<FInventoryPropertiesBase>()->TotalSlotsOverride;
		DesiredNumSlots = FMath::Min(0, DesiredNumSlots);
		
		const int32 InitialNumSlots = DesiredNumSlots;

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

		ensureMsgf((InitialNumSlots >= 0) && (DesiredNumSlots >= DesiredNumSlots),
			TEXT("Lol, we have a skill-issue error here. InitialNumSlots: %d, DesiredNumSlots: %d"),
			InitialNumSlots, DesiredNumSlots);
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
}

void ASlottableInventory::BindToInventoryDelegates(AInventory* MutableParent)
{
	check(MutableParent);

	MutableParent->OnItemAddedDelegate.AddUObject(this, &ThisClass::Event_OnItemAdded);
	MutableParent->OnItemRemovedDelegate.AddUObject(this, &ThisClass::Event_OnItemRemoved);
}

void ASlottableInventory::Event_OnItemAdded(const FInventoryChangeMessage& Payload)
{
	ITEMIZATION_WARN("Event_OnItemAdded");
	
	if (!ensure(Payload.ItemEntry))
	{
		return;
	}

	// Testing only !!!!!!!!
	FInventorySlotHandle SlotHandle = GetNextFreeItemSlot();
	if (SlotHandle.IsValid())
	{
		FInventorySlotEntry& Slot = InventorySlots[SlotHandle.GetSlotId()];
		Slot.GetHandle_Ref().SetUID(Payload.ItemEntry->ItemHandle.Get());

		ITEMIZATION_WARN("Event_OnItemAdded: %s %s", *Slot.ToString(), *Payload.ItemEntry->ItemHandle.ToString());

		InventorySlots.MarkItemDirty(Slot);
	}
}

void ASlottableInventory::Event_OnItemRemoved(const FInventoryChangeMessage& Payload)
{
}

FInventorySlotHandle ASlottableInventory::GetNextFreeItemSlot(FInventorySlotHandle Start) const
{
	int32 StartingIndex = 0;
	if (Start.IsSlotValid())
	{
		StartingIndex = Start.GetSlotId();
	}

	for (int i = StartingIndex; i < InventorySlots.Slots.Num(); ++i)
	{
		const FInventorySlotEntry& Slot = InventorySlots[i];

		// Skip occupied and disabled slots 
		if (Slot.IsOccupied() || !Slot.IsEnabled())
		{
			ITEMIZATION_ERROR("Skipping %s / %s slot: %s",
				Slot.IsOccupied() ? TEXT("Occupied") : TEXT("Empty"),
				Slot.IsEnabled() ? TEXT("Enabled") : TEXT("Disabled"),
				*Slot.ToString());
			continue;
		}

		return Slot.GetHandle();
	}

	return FInventorySlotHandle();
}


void ASlottableInventory::OnRep_InventorySlots()
{
}
