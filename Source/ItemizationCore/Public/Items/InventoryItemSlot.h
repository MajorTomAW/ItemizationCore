// Author: Tom Werner (MajorT), 2025

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "InventoryItemHandle.h"
#include "InventorySlotHandle.h"
#include "ItemizationCoreMacros.h"
#include "Net/Serialization/FastArraySerializer.h"

#include "InventoryItemSlot.generated.h"


class AInventoryBase;

#define UE_API ITEMIZATIONCORE_API

/** Fast array serializer item for a single item slot in an inventory. */
USTRUCT(BlueprintType, MinimalAPI)
struct FInventoryItemSlot : public FFastArraySerializerItem
{
	GENERATED_BODY()
	friend struct FInventorySlotContainer;
	friend class AInventoryBase;

public:
	FInventoryItemSlot() = default;

	/** Returns this item slot as a debug string. */
	FString GetDebugString() const;

	/** Returns the slot group tag. */
	const FGameplayTag& GetGroupTag() const
	{
		return GroupTag;
	}

	/** Returns the slot tags. */
	const FGameplayTagContainer& GetSlotTags() const
	{
		return SlotTags;
	}

	/** Returns true if this slot has a slot tag. */
	bool HasSlotTag(const FGameplayTag& SlotTag) const
	{
		return SlotTags.HasTagExact(SlotTag);
	}

	/** Returns true if this slot has any of the given slot tags. */
	bool HasAnySlotTags(const FGameplayTagContainer& SlotTagsToCheck) const
	{
		return SlotTags.HasAnyExact(SlotTagsToCheck);
	}

	/** Returns the slot handle of this slot. */
	const FInventorySlotHandle& GetSlotHandle() const
	{
		return SlotHandle;
	}

	/** Returns the row index of this slot. */
	uint32 GetRowIndex() const
	{
		return SlotHandle.GetRowIndex();
	}

	/** Returns the column index of this slot. */
	uint32 GetColumnIndex() const
	{
		return SlotHandle.GetColumnIndex();
	}

	/** Returns the item handle (can often be an invalid handle). */
	const FInventoryItemHandle& GetItemHandle() const
	{
		return ItemHandle;
	}

	/** Returns true if this slot is unoccupied. */
	bool IsUnoccupied() const
	{
		// Count as unoccupied when no item handle is set
		return !ItemHandle.IsValid();
	}

	/** Places an item in this slot. */
	void SetItemHandle(const FInventoryItemHandle& NewItemHandle)
	{
		ItemHandle = NewItemHandle;
	}

	/** Sets the group tag. */
	void SetGroupTag(const FGameplayTag& NewGroupTag)
	{
		GroupTag = NewGroupTag;
	}

	/** Sets the slot tags. */
	void SetSlotTags(const FGameplayTagContainer& NewSlotTags)
	{
		SlotTags = NewSlotTags;
	}

	/** Sets the slot handle. */
	void SetSlotHandle(const FInventorySlotHandle& NewSlotHandle)
	{
		SlotHandle = NewSlotHandle;
	}

private:
	/** The slot group tag. */
	UPROPERTY()
	FGameplayTag GroupTag;

	/** Additional tags for this slot. */
	UPROPERTY()
	FGameplayTagContainer SlotTags;

	/** The unique handle to this slot for outside references. */
	UPROPERTY()
	FInventorySlotHandle SlotHandle;

	/** The unique handle to the item in this slot. */
	UPROPERTY()
	FInventoryItemHandle ItemHandle;

public:
	bool operator==(const FInventoryItemSlot& Other) const
	{
		return SlotHandle == Other.SlotHandle;
	}

	bool operator==(const FInventorySlotHandle& OtherSlotHandle) const
	{
		return SlotHandle == OtherSlotHandle;
	}

	bool operator==(const FInventoryItemHandle& OtherItemHandle) const
	{
		return ItemHandle == OtherItemHandle;
	}

	bool operator<(const FInventoryItemSlot& Other) const
	{
		return SlotHandle < Other.SlotHandle;
	}
	bool operator>(const FInventoryItemSlot& Other) const
	{
		return !operator<(Other);
	}
};

template<>
struct TStructOpsTypeTraits<FInventoryItemSlot> : TStructOpsTypeTraitsBase2<FInventoryItemSlot>
{
	enum
	{
		WithIdenticalViaEquality = true,
		WithNetSharedSerialization = true,
	};
};

/** Fast array serializer for a list of item slots in an inventory. */
USTRUCT(BlueprintType, MinimalAPI)
struct FInventorySlotContainer : public FFastArraySerializer
{
	GENERATED_BODY()

public:
	UE_API FInventorySlotContainer();
	UE_API FInventorySlotContainer(AInventoryBase* InOwningInventory);

	/** Tries to find an FInventoryItemSlot by its handle. */
	UE_API FInventoryItemSlot* FindItemSlotByHandle(const FInventorySlotHandle& SlotHandle) const;

	/** Tries to find an FInventoryItemSlot by an item handle. */
	UE_API FInventoryItemSlot* FindItemSlotByHandle(const FInventoryItemHandle& ItemHandle) const;

	/** Returns all item slots in the given group. */
	UE_API TArray<FInventoryItemSlot*> FindSlotsInGroup(const FGameplayTag& InGroupTag) const;

	/** Returns all item handles in the given group. */
	UE_API TArray<FInventoryItemHandle> FindItemHandlesInGroup(const FGameplayTag& InGroupTag) const;

	/** Returns all item handles. */
	UE_API TArray<FInventoryItemHandle> GetAllItemHandles() const;

	/** Tries to find the next free item slop in the given group. */
	UE_API FInventoryItemSlot* GetNextUnoccupiedItemSlotInGroup(const FGameplayTag& InGroupTag) const;

	/** Finds all slot groups. */
	UE_API TArray<FGameplayTag> GetAllItemGroups() const;

	//~ Begin FFastArraySerializer Interface
	bool NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParms)
	{
		return FFastArraySerializer::FastArrayDeltaSerialize<FInventoryItemSlot, FInventorySlotContainer>(ItemSlots, DeltaParms, *this);
	}
	//~ End FFastArraySerializer Interface

	/** TArray accessors for this container. */
	CREATE_ARRAY_SERIALIZER_TARRAY_ACCESSORS(FInventorySlotContainer, FInventoryItemSlot, ItemSlots);

public:
	/** List of all item slots, should be preallocated at the inventory's creation. */
	UPROPERTY()
	TArray<FInventoryItemSlot> ItemSlots;

	/** The Inventory class that owns this list. */
	UPROPERTY(NotReplicated)
	TObjectPtr<AInventoryBase> OwningInventory;

	//@TODO: Want to find a way not having to query all group tags just to find all slots in a group
	/*/** Faster lookup of item slots per inventory group. #1#
	UPROPERTY(NotReplicated)
	TMap<FGameplayTag, TArray<FInventoryItemSlot>> ItemSlotLookupMap;*/
};

template<>
struct TStructOpsTypeTraits<FInventorySlotContainer> : TStructOpsTypeTraitsBase2<FInventorySlotContainer>
{
	enum
	{
		WithNetDeltaSerializer = true,
		WithNetSharedSerialization = true,
	};
};

#undef UE_API
