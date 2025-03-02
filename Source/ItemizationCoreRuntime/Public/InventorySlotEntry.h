// Copyright © 2024 Playton. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "InventoryItemEntryHandle.h"
#include "Net/Serialization/FastArraySerializer.h"

// For swapping
#include <algorithm>

#include "InventorySlotEntry.generated.h"

class UItemDefinition;
class UInventoryItemInstance;
class UInventorySlotManager;
class UInventoryManager;
struct FInventoryItemEntry;
struct FInventorySlotContainer;

/**
 * Fast array serializer for inventory slot entries.
 */
USTRUCT(BlueprintType)
struct ITEMIZATIONCORERUNTIME_API FInventorySlotEntry : public FFastArraySerializerItem
{
	GENERATED_BODY()

public:
	friend struct FInventoryItemEntry;
	friend struct FInventorySlotContainer;
	friend class UInventoryItemInstance;
	friend class UInventoryManager;

	FInventorySlotEntry();
	explicit FInventorySlotEntry(const FInventoryItemEntryHandle& InHandle, int32 InSlotIndex = INDEX_NONE);

protected:
	/** The item entry that this slot entry represents. */
	UPROPERTY()
	FInventoryItemEntryHandle Handle;

	/** The unique identifier for this slot. */
	UPROPERTY()
	int32 SlotID;
	
	/** The tag associated with this slot. Can be used for filtering. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Slot)
	FGameplayTag SlotTag;

	/**
	 * In the future, this could be extended to support more logic such as slot size, etc.
	 */

public:
	// ----------------------------------------------------------------------------------------------------------------
	// Operators
	// ----------------------------------------------------------------------------------------------------------------

	FORCEINLINE bool operator==(const FInventorySlotEntry& Other) const
	{
		return (SlotTag == Other.SlotTag) && (SlotID == Other.SlotID) /*&& (Handle == Other.Handle)*/;
	}
	FORCEINLINE bool operator==(const int32& OtherID) const
	{
		return SlotID == OtherID;
	}
	FORCEINLINE bool operator==(const FInventoryItemEntryHandle& OtherHandle) const
	{
		return Handle == OtherHandle;
	}

	FORCEINLINE bool operator!=(const FInventorySlotEntry& Other) const
	{
		return !(*this == Other);
	}
	FORCEINLINE bool operator!=(const int32& OtherID) const
	{
		return !(*this == OtherID);
	}
	FORCEINLINE bool operator!=(const FInventoryItemEntryHandle& OtherHandle) const
	{
		return !(*this == OtherHandle);
	}

	/** Returns true if this slot entry is valid. Aka, it has an item assigned to it. */
	FORCEINLINE bool IsValid() const
	{
		return Handle.IsValid();
	}

	/** Returns the handle of the item in this slot. */
	FORCEINLINE const FInventoryItemEntryHandle& GetHandle() const
	{
		return Handle;
	}
	FORCEINLINE FInventoryItemEntryHandle& GetHandle_Ref()
	{
		return Handle;
	}

	/** Returns the slot ID of this slot. */
	FORCEINLINE int32 GetSlotID() const
	{
		return SlotID;
	}
	FORCEINLINE int32& GetSlotID_Ref()
	{
		return SlotID;
	}

	/** Returns the slot tag of this slot. */
	FORCEINLINE const FGameplayTag& GetSlotTag() const
	{
		return SlotTag;
	}
	FORCEINLINE FGameplayTag& GetSlotTag_Ref()
	{
		return SlotTag;
	}

	/** Displays this slot entry as a string. */
	FORCEINLINE FString ToString() const
	{
		return FString::Printf(TEXT("Slot[%d][%s]: %s")
			, SlotID, *SlotTag.ToString(), *Handle.ToString());
	}

public:
	//~ Begin FFastArraySerializerItem Interface
	void PreReplicatedRemove(const FInventorySlotContainer& InArraySerializer);
	void PostReplicatedAdd(const FInventorySlotContainer& InArraySerializer);
	void PostReplicatedChange(const FInventorySlotContainer& InArraySerializer);
	//~ End FFastArraySerializerItem Interface
};

/** Fast array serializer for inventory slot entries. */
USTRUCT(BlueprintType)
struct ITEMIZATIONCORERUNTIME_API FInventorySlotContainer : public FFastArraySerializer
{
	GENERATED_BODY()

public:
	FInventorySlotContainer();
	FInventorySlotContainer(UInventorySlotManager* InOwner);

	//~ Begin FFastArraySerializer Interface
	void PreReplicatedRemove(const TArrayView<int32> RemovedIndices, int32 FinalSize);
	void PostReplicatedAdd(const TArrayView<int32> AddedIndices, int32 FinalSize);
	void PostReplicatedChange(const TArrayView<int32> ChangedIndices, int32 FinalSize);

	bool NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParms)
	{
		return FFastArraySerializer::FastArrayDeltaSerialize<FInventorySlotEntry, FInventorySlotContainer>(Slots, DeltaParms, *this);
	}
	//~ End FFastArraySerializer Interface

public:
	template <typename T>
	FORCEINLINE FInventorySlotEntry* Find(const T& Operator)
	{
		for (FInventorySlotEntry& Entry : Slots)
		{
			if (Entry == Operator)
			{
				return &Entry;	
			}
		}
		
		return nullptr;
	}
	template <typename T>
	FORCEINLINE const FInventorySlotEntry* Find(const T& Operator) const
	{
		for (const FInventorySlotEntry& Entry : Slots)
		{
			if (Entry == Operator)
			{
				return &Entry;	
			}
		}
		return nullptr;
	}

	/** Swaps to slot entries. */
	void Swap(int32 IndexA, int32 IndexB)
    {
        // Swap the handles but keep the slot IDs
		std::swap(Slots[IndexA].Handle, Slots[IndexB].Handle);
    }

public:
	/** List of inventory slot entries. */
	UPROPERTY()
	TArray<FInventorySlotEntry> Slots;

	/** Component that owns this list. */
	UPROPERTY(NotReplicated)
	TObjectPtr<UInventorySlotManager> Owner;
};

template<>
struct TStructOpsTypeTraits<FInventorySlotContainer> : public TStructOpsTypeTraitsBase2<FInventorySlotContainer>
{
	enum
	{
		WithNetDeltaSerializer = true,
	};
};