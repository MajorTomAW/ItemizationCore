// Copyright © 2025 MajorT. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "StructUtils/InstancedStruct.h"

#include "ItemComponentData.generated.h"

struct FInventoryItemTransactionBase;
struct FInventoryItemEntry;
enum class EItemComponentInstancingPolicy : uint8;
class UObject;

/** 
 * This struct represents the base item component data.
 * It can be used to define custom logic for items.
 * These are stored as instanced structs in the inventory system and have logic that can be called during runtime.
 */
USTRUCT(BlueprintType, Blueprintable)
struct ITEMIZATIONCORERUNTIME_API FItemComponentData
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

	/** Optional to add tags to the asset registry context for the owning item definition. */
	virtual void GetAssetRegistryTags(FAssetRegistryTagsContext Context) const;

#if WITH_EDITOR
	/** Validates the item component data. */
	virtual EDataValidationResult IsDataValid(FDataValidationContext& Context) const;
#endif

	/**
	 * Called before an Item Entry is added or removed from an inventory.
	 * This is essential to fill in any important data in the un-initialized Item Entry.
	 */
	virtual void EvaluateItemEntry(FInventoryItemEntry& ItemEntry, const FInventoryItemTransactionBase& Transaction) const;

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

/**
 * Instanced struct for the item component data.
 * This allows identifying components based on GUID in the editor (even empty ones).
 */
USTRUCT(BlueprintType)
struct ITEMIZATIONCORERUNTIME_API FItemComponentDataInstance
{
	GENERATED_BODY()

	FItemComponentDataInstance() = default;

	template <typename T, typename std::enable_if_t<std::is_base_of_v<FItemComponentData, std::decay_t<T>>>>
	static FItemComponentDataInstance Make(const T& Struct)
	{
		FItemComponentDataInstance NewInstance;
		NewInstance.Component.InitializeAsScriptStruct(TBaseStructure<T>::Get(), reinterpret_cast<const uint8*>(&Struct));
#if WITH_EDITORONLY_DATA
		NewInstance.ComponentGuid = FGuid::NewGuid();
#endif
		return NewInstance;
	}

	/** The actual item component data. */
	UPROPERTY(EditDefaultsOnly, Category=Component, meta=(ExcludeBaseStruct,ShowOnlyInnerProperties), NoClear)
	TInstancedStruct<FItemComponentData> Component;

#if WITH_EDITORONLY_DATA
	/** The GUID of this item component data. */
	UPROPERTY(EditDefaultsOnly, Category=Component, meta=(EditConditionHides,EditCondition=false))
	FGuid ComponentGuid;
#endif
};