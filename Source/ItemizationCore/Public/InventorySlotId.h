// Author: Tom Werner (MajorT), 2025

#pragma once

#include "InventorySlotId.generated.h"

USTRUCT(BlueprintType)
struct alignas(8) FInventorySlotId
{
	GENERATED_BODY()

	FInventorySlotId() = default;
	FInventorySlotId(uint32 InRowIndex, uint32 InColumnIndex)
		: RowIndex(InRowIndex)
		, ColumnIndex(InColumnIndex)
	{
	}

public:
	enum : uint32
	{
		INVALID_SLOT = UINT32_MAX,
	};

	/** Returns this handles row index. */
	[[nodiscard]] uint32 GetRowIndex() const
	{
		return RowIndex;
	}

	/** Returns this handles column index. */
	[[nodiscard]] uint32 GetColumnIndex() const
	{
		return ColumnIndex;
	}

	/** Converts this handle to a string. */
	[[nodiscard]] FString ToString() const
	{
		return IsValid() ? FString::Printf(TEXT("(Row: %u Column: %u)"), RowIndex, ColumnIndex) : TEXT("NullHandle");
	}

	/** Resets this handle to an invalid state. */
	void Reset()
	{
		RowIndex = INVALID_SLOT;
		ColumnIndex = INVALID_SLOT;
	}

	/** Checks if this handle is valid (not pointing to an invalid slot). */
	bool IsValid() const
	{
		return (RowIndex != INVALID_SLOT) && (ColumnIndex != INVALID_SLOT);
	}

	static FInventorySlotId InvalidId;

public:
	/** Compared this handle with another handle. */
	bool operator==(const FInventorySlotId& Other) const
	{
		return GetRowIndex() == Other.GetRowIndex() && GetColumnIndex() == Other.GetColumnIndex();
	}
	bool operator!=(const FInventorySlotId& Other) const
	{
		return !operator==(Other);
	}

	/** For sorting purposes. */
	bool operator<(const FInventorySlotId& Other) const
	{
		return GetRowIndex() < Other.GetRowIndex() && GetColumnIndex() < Other.GetColumnIndex();
	}
	bool operator>(const FInventorySlotId& Other) const
	{
		return !operator<(Other);
	}

	/** Archive operator for serialization. */
	friend FArchive& operator<<(FArchive& Ar, FInventorySlotId& Slot)
	{
		Ar << Slot.RowIndex;
		Ar << Slot.ColumnIndex;
		return Ar;
	}

	/** Returns a has value for this handle. */
	friend uint32 GetTypeHash(const FInventorySlotId& Slot)
	{
		return HashCombine(::GetTypeHash(Slot.RowIndex), ::GetTypeHash(Slot.ColumnIndex));
	}
	uint32 GetHash() const
	{
		return GetTypeHash(*this);
	}

	explicit operator bool() const
	{
		return IsValid();
	}

private:
	/** Pointer to the row that the item lives in. INT32_MAX means its invalid. */
	UPROPERTY()
	uint32 RowIndex = INVALID_SLOT;

	/** Pointer to the column that the item lives in. INT32_MAX means its invalid. */
	UPROPERTY()
	uint32 ColumnIndex = INVALID_SLOT;
};

static_assert(sizeof(FInventorySlotId) == sizeof(uint64), "Expected FInventorySlotId to be 8 bytes.");
static_assert(alignof(FInventorySlotId) == sizeof(uint64), "Expected FInventorySlotId to be aligned to 8 bytes.");
