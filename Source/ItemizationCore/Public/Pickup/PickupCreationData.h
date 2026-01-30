// Author: Tom Werner (dc: majort), 2026 January

#pragma once
#include "Items/InventoryItemEntry.h"

#include "PickupCreationData.generated.h"

/**
 * Base pickup data implementation struct.
 * It's highly recommended that you create your own pickup item data from FItemComponentData_PickupBase
 * and override its GetPickupCreationData() function to implement your own pickup logic.
 */
USTRUCT(BlueprintType)
struct FPickupCreationData_Impl
{
	GENERATED_BODY()

public:
	/** Snapshot of the item entry we're trying to put into the pickup. */
	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category=Data)
	FInventoryItemEntry PickupItemEntry;

	/** Location to spawn the pickup actor at. */
	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category=Data)
	FVector Location = FVector::ZeroVector;

	/** Rotation to spawn the pickup actor with. */
	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category=Data)
	FRotator Rotation = FRotator::ZeroRotator;

	/** Optional pickup instigator. */
	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category=Data)
	TWeakObjectPtr<AActor> OptionalInstigator;

	/** Optional class to override the pickup actor being spawned. */
	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category=Data, meta=(MustImplement="/Script/ItemizationCore.PickupableInterface"))
	TSubclassOf<AActor> OverridePickupActorClass;
};

/** Data wrapper for pickup actor creation. */
USTRUCT(BlueprintType)
struct FPickupCreationData
{
	GENERATED_BODY()

public:
	/** Returns the data. */
	template <typename T UE_REQUIRES(std::is_base_of_v<FPickupCreationData_Impl, T>)>
	const T* Get() const
	{
		return Data.GetPtr<T>();
	}

	/** Returns the mutable data. */
	template <typename T UE_REQUIRES(std::is_base_of_v<FPickupCreationData_Impl, T>)>
	T& GetMutable()
	{
		return Data.GetMutable<T>();
	}

	/** Sets the data. */
	template <typename T UE_REQUIRES(std::is_base_of_v<FPickupCreationData_Impl, T>)>
	void Set(const T& InData)
	{
		Data = TInstancedStruct<T>::Make(InData);
	}

	/** Returns true if this creation data is valid. */
	bool IsValid() const
	{
		return Data.IsValid();
	}

protected:
	/** Implementation of the pickup creation data. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Data)
	TInstancedStruct<FPickupCreationData_Impl> Data;
};