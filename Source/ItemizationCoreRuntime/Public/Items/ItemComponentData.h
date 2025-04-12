// Copyright © 2025 MajorT. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "ItemComponentData.generated.h"

enum class EItemComponentInstancingPolicy : uint8;
class UObject;

/** 
 * This struct represents the base item component data.
 * It can be used to define custom logic for items.
 * These are stored as instanced structs in the inventory system and have logic that can be called during runtime.
 */
USTRUCT(BlueprintType, Blueprintable)
struct FItemComponentData
{
	GENERATED_BODY()

public:
	FItemComponentData();
	virtual ~FItemComponentData() = default;

	/** Returns how this item component data is meant to be instanced. */
	FORCEINLINE EItemComponentInstancingPolicy GetInstancingPolicy() const
	{
		return InstancingPolicy;
	}

	/** For instanced structs only, need to be serialized with the item. */
	virtual bool NetSerialize(FArchive& Ar, UPackageMap* Map, bool& bOutSuccess);

protected:
	/** The instancing policy of this item component data. */
	EItemComponentInstancingPolicy InstancingPolicy;
};

template<>
struct TStructOpsTypeTraits<FItemComponentData> : public TStructOpsTypeTraitsBase2<FItemComponentData>
{
	enum
	{
		WithNetSerializer = true,
	};
};