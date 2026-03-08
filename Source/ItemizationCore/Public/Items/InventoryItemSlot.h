// Author: Tom Werner (MajorT), 2025

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "InventoryItemId.h"
#include "InventorySlotId.h"
#include "ItemizationCoreMacros.h"
#include "Net/Serialization/FastArraySerializer.h"

#include "InventoryItemSlot.generated.h"


class IInventoryItemInstanceInterface;
class ASlottableInventory;
class AInventoryBase;
class UObject;
struct FInventoryItemEntry;


#define UE_API ITEMIZATIONCORE_API

/** Fast array serializer item for a single item slot in an inventory. */
USTRUCT(BlueprintType, MinimalAPI)
struct FInventoryItemSlot : public FFastArraySerializerItem
{
	GENERATED_BODY()
	friend struct FInventorySlotList;
	friend class AInventoryBase;

public:
	UE_API FInventoryItemSlot();

	/** Returns this item slot as a debug string. */
	UE_API FString GetDebugString() const;

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

	/** Returns true if this slot is occupied. */
	bool IsOccupied() const
	{
		return ItemId.IsValid();
	}

	/** Returns true if this slot is unoccupied. */
	bool IsUnoccupied() const
	{
		return !IsOccupied();
	}

	/** Places an item in this slot. */
	UE_API void OccupySlot(const FInventoryItemEntry& ItemEntry);

	/** Clears an item from this slot. */
	UE_API void UnoccupySlot();

	/** Returns the item instance in this slot. */
	UE_API UObject* GetItemInSlot() const;
	template <class ItemType>
	ItemType* GetItemInSlot() const
	{
		return CastChecked<ItemType>(GetItemInSlot(), ECastCheckedType::NullAllowed);
	}

	/** Returns the item entry in this slot. */
	UE_API const FInventoryItemEntry* GetItemEntryInSlot() const;
	UE_API FInventoryItemEntry* GetItemEntryInSlot();

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

	/** Marks this slot dirty. */
	UE_API void MarkSlotDirty();

	/** Attempts to resolve the item instance. */
	UE_API void TryResolveItemInstance();

	/** Swaps this slots contents with the contents of the given slot.
	 * Note that this will only swap the item id (and item instance), but not the slot id or its tags.
	 */
	void SwapContents(FInventoryItemSlot& Other);

	/** Returns the owning inventory of this item. */
	ASlottableInventory* GetOwningInventory() const { return OwningInventory.Get(); }

	/** Sets the owning inventory of this item. */
	UE_API void SetOwningInventory(ASlottableInventory* InOwningInventory);

	//~ Begin FFastArraySerializerItem Interface
	void PreReplicatedRemove(const FInventorySlotList& InArraySerializer);
	void PostReplicatedAdd(const FInventorySlotList& InArraySerializer);
	void PostReplicatedChange(const FInventorySlotList& InArraySerializer);
	//~ End FFastArraySerializerItem Interface

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

	/** Non replicated item instance that lives inside this slot. Gets resolved when item id is assigned. */
	/*UPROPERTY(NotReplicated)
	TWeakObjectPtr<UObject> ItemInstance;*/
	uint8 bWaitingOnItemInstance:1 = false;

	/** Reference to the owning inventory. */
	UPROPERTY(NotReplicated)
	TWeakObjectPtr<ASlottableInventory> OwningInventory;

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
	friend struct FInventoryItemSlot;
	friend class ASlottableInventory;

public:
	UE_API FInventorySlotList();
	UE_API FInventorySlotList(ASlottableInventory* InOwningInventory);

	/** Adds an item slot to this list. */
	UE_API FInventoryItemSlot& AddSlotToList(FInventoryItemSlot ItemSlot);
	UE_API FInventoryItemSlot& AddSlotToList_Defaulted(const FGameplayTag& SlotGroup);

	/** Removes an item slot from this list. */
	UE_API bool RemoveSlotFromList(FInventorySlotId SlotId);

	/** Swaps the contents of two item slots. */
	UE_API void SwapSlotsContent(const FInventorySlotId& SlotIdA, const FGameplayTag& SlotGroupA, const FInventorySlotId& SlotIdB, const FGameplayTag& SlotGroupB) const;

	/** Tries to find an FInventoryItemSlot by its item instance. */
	UE_API FInventoryItemSlot* FindItemSlot(const TScriptInterface<IInventoryItemInstanceInterface>& ItemInstance) const;

	/** Tries to find an FInventoryItemSlot by its Id. */
	UE_API FInventoryItemSlot* FindItemSlotBySlotId(const FInventorySlotId& SlotId, const FGameplayTag& GroupTag = FGameplayTag()) const;

	/** Tries to find an FInventoryItemSlot by an item Id. */
	UE_API FInventoryItemSlot* FindItemSlotByItemId(const FInventoryItemId& ItemId, const FGameplayTag& GroupTag = FGameplayTag()) const;

	/** Returns all item slots in the given group. */
	UE_API TArray<FInventoryItemSlot*> GetItemSlotsInGroup(const FGameplayTag& GroupTag);
	UE_API TArray<const FInventoryItemSlot*> GetItemSlotsInGroup(const FGameplayTag& GroupTag) const;

	/** Returns all item Ids in the given group. */
	UE_API TArray<FInventoryItemId> GetItemIdsInGroup(const FGameplayTag& InGroupTag) const;

	/** Returns all item Ids. */
	UE_API TArray<FInventoryItemId> GetAllItemIds() const;

	/** Returns the inventory group of the given Item id. */
	UE_API FGameplayTag GetInventoryGroupForItemId(const FInventoryItemId& ItemId) const;

	/** Tries to find the next free item slot in the given group. */
	UE_API FInventoryItemSlot* GetNextUnoccupiedSlotInGroup(const FGameplayTag& InGroupTag) const;

	/** Tries to find the next free item slot id in the given group. */
	UE_API FInventorySlotId GetNextUnoccupiedSlotIdInGroup(const FGameplayTag& InGroupTag) const;

	/** Finds all slot groups. */
	UE_API TArray<FGameplayTag> GetAllItemGroupTags() const;

	//~ Begin FFastArraySerializer Interface
	UE_API void PreReplicatedRemove(const TArrayView<int32> RemovedIndices, int32 FinalSize);
	UE_API void PostReplicatedAdd(const TArrayView<int32> AddedIndices, int32 FinalSize);
	UE_API void PostReplicatedChange(const TArrayView<int32> ChangedIndices, int32 FinalSize);

	bool NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParms)
	{
		return FFastArraySerializer::FastArrayDeltaSerialize<FInventoryItemSlot, FInventorySlotList>(ItemSlots, DeltaParms, *this);
	}
	//~ End FFastArraySerializer Interface

	/** TArray accessors for this container. */
	CREATE_ARRAY_SERIALIZER_TARRAY_ACCESSORS(FInventorySlotList, FInventoryItemSlot, ItemSlots);

private:
	/** List of all item slots, should be preallocated at the inventory's creation. */
	UPROPERTY()
	TArray<FInventoryItemSlot> ItemSlots;

	/** The Inventory class that owns this list. */
	UPROPERTY(NotReplicated)
	TObjectPtr<ASlottableInventory> OwningInventory;
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
