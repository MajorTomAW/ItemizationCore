// Author: Tom Werner (MajorT), 2025

#pragma once

#include "CoreMinimal.h"
#include "StructUtils/InstancedStruct.h"

#include "ItemComponentData.generated.h"

struct FInventoryItemTransactionBase;
struct FInventoryItemEntry;
class UObject;
class UWorld;
struct FFrame;

/** 
 * This struct represents the base item component data.
 * It can be used to define custom logic for items.
 * These are stored as instanced structs in the inventory system and have logic that can be called during runtime.
 */
USTRUCT(BlueprintType)
struct ITEMIZATIONCORERUNTIME_API FItemComponentData
{
	GENERATED_BODY()

public:
	FItemComponentData();
	virtual ~FItemComponentData() = default;

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

	/**
	 * Checks if two item entries can be merged into a single stack.
	 * In this case, 'ThisEntry' will be merged into 'OtherEntry'.
	 */
	virtual bool CanMergeItems(const FInventoryItemEntry& ThisEntry, const FInventoryItemEntry& OtherEntry) const;
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

	/** Initializes a new item component data instance. */
	template <typename T>
	requires std::is_base_of_v<FItemComponentData, std::decay_t<T>>
	static FItemComponentDataInstance Make(const T& Struct)
	{
		FItemComponentDataInstance NewInstance;
		NewInstance.Component.InitializeAsScriptStruct(TBaseStructure<T>::Get(), reinterpret_cast<const uint8*>(&Struct));
#if WITH_EDITORONLY_DATA
		NewInstance.ComponentGuid = FGuid::NewGuid();
#endif
		return NewInstance;
	}

	/** Checks whether this item component data is of a specified type. */
	template <typename T>
	requires std::is_base_of_v<FItemComponentData, std::decay_t<T>>
	bool IsOfType() const
	{
		return IsValid() && Component.GetScriptStruct() == TBaseStructure<T>::Get();
	}
	bool IsOfType(const UScriptStruct* Struct) const
	{
		return IsValid() && Component.GetScriptStruct() == Struct;
	}

	/** Returns the item component data as a specific type. */
	template <typename T>
	requires std::is_base_of_v<FItemComponentData, std::decay_t<T>>
	const T* GetComponent() const
	{
		return Component.GetPtr<T>();
	}

	/** Returns true if this item component data is valid and has been initialized. */
	bool IsValid() const { return Component.IsValid(); }

	/** The actual item component data. */
	UPROPERTY(EditDefaultsOnly, Category=Component, meta=(ExcludeBaseStruct,ShowOnlyInnerProperties), NoClear)
	TInstancedStruct<FItemComponentData> Component;

#if WITH_EDITORONLY_DATA
	/** The GUID of this item component data. */
	UPROPERTY(EditDefaultsOnly, Category=Component, meta=(EditConditionHides,EditCondition=false))
	FGuid ComponentGuid;
#endif
};