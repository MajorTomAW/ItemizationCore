// Author: Tom Werner (MajorT), 2025

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "InventoryItemId.h"
#include "InventorySlotId.h"
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
	friend struct FInventorySlotList;
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

	/** Returns the slot Id of this slot. */
	const FInventorySlotId& GetSlotId() const
	{
		return SlotId;
	}

	/** Returns the row index of this slot. */
	uint32 GetRowIndex() const
	{
		return SlotId.GetRowIndex();
	}

	/** Returns the column index of this slot. */
	uint32 GetColumnIndex() const
	{
		return SlotId.GetColumnIndex();
	}

	/** Returns the item Id (can often be an invalid Id). */
	const FInventoryItemId& GetItemId() const
	{
		return ItemId;
	}

	/** Returns true if this slot is unoccupied. */
	bool IsUnoccupied() const
	{
		// Count as unoccupied when no item Id is set
		return !ItemId.IsValid();
	}

	/** Places an item in this slot. */
	void SetItemId(const FInventoryItemId& NewItemId)
	{
		ItemId = NewItemId;
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

	/** Sets the slot Id. */
	void SetSlotId(const FInventorySlotId& NewSlotId)
	{
		SlotId = NewSlotId;
	}

private:
	/** The slot group tag. */
	UPROPERTY()
	FGameplayTag GroupTag;

	/** Additional tags for this slot. */
	UPROPERTY()
	FGameplayTagContainer SlotTags;

	/** The unique Id to this slot for outside references. */
	UPROPERTY()
	FInventorySlotId SlotId;

	/** The unique Id to the item in this slot. */
	UPROPERTY()
	FInventoryItemId ItemId;

public:
	bool operator==(const FInventoryItemSlot& Other) const
	{
		return SlotId == Other.SlotId;
	}

	bool operator==(const FInventorySlotId& OtherSlotId) const
	{
		return SlotId == OtherSlotId;
	}

	bool operator==(const FInventoryItemId& OtherItemId) const
	{
		return ItemId == OtherItemId;
	}

	bool operator<(const FInventoryItemSlot& Other) const
	{
		return SlotId < Other.SlotId;
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
struct FInventorySlotList : public FFastArraySerializer
{
	GENERATED_BODY()

public:
	UE_API FInventorySlotList();
	UE_API FInventorySlotList(AInventoryBase* InOwningInventory);

	/** Tries to find an FInventoryItemSlot by its Id. */
	UE_API FInventoryItemSlot* FindItemSlotById(const FInventorySlotId& SlotId) const;

	/** Tries to find an FInventoryItemSlot by an item Id. */
	UE_API FInventoryItemSlot* FindItemSlotById(const FInventoryItemId& ItemId) const;

	/** Returns all item slots in the given group. */
	UE_API TArray<FInventoryItemSlot*> FindSlotsInGroup(const FGameplayTag& InGroupTag) const;

	/** Returns all item Ids in the given group. */
	UE_API TArray<FInventoryItemId> FindItemIdsInGroup(const FGameplayTag& InGroupTag) const;

	/** Returns all item Ids. */
	UE_API TArray<FInventoryItemId> GetAllItemIds() const;

	/** Tries to find the next free item slop in the given group. */
	UE_API FInventoryItemSlot* GetNextUnoccupiedItemSlotInGroup(const FGameplayTag& InGroupTag) const;

	/** Finds all slot groups. */
	UE_API TArray<FGameplayTag> GetAllItemGroups() const;

	//~ Begin FFastArraySerializer Interface
	bool NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParms)
	{
		return FFastArraySerializer::FastArrayDeltaSerialize<FInventoryItemSlot, FInventorySlotList>(ItemSlots, DeltaParms, *this);
	}
	//~ End FFastArraySerializer Interface

	/** TArray accessors for this container. */
	CREATE_ARRAY_SERIALIZER_TARRAY_ACCESSORS(FInventorySlotList, FInventoryItemSlot, ItemSlots);

public:
	/** List of all item slots, should be preallocated at the inventory's creation. */
	UPROPERTY()
	TArray<FInventoryItemSlot> ItemSlots;

	/** The Inventory class that owns this list. */
	UPROPERTY(NotReplicated)
	TObjectPtr<AInventoryBase> OwningInventory;
	
	/** Faster lookup of item slots per inventory group. */
	TMap<FGameplayTag, TArray<FInventoryItemSlot>> ItemSlotLookupMap;
};

template<>
struct TStructOpsTypeTraits<FInventorySlotList> : TStructOpsTypeTraitsBase2<FInventorySlotList>
{
	enum
	{
		WithNetDeltaSerializer = true,
		WithNetSharedSerialization = true,
	};
};

#undef UE_API
