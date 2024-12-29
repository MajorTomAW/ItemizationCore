// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InstancedStruct.h"
#include "InventoryItemEntry.h"
#include "ItemizationCoreTypes.h"

#include "ItemComponentData.generated.h"

struct FInventoryItemEntry;

/** Custom metadata for item components. */
namespace UM
{
	enum
	{
		// [StructMetadata] Indicates that the instanced struct should act like a TSet.
		SingletonComponent,

		// [StructMetadata] Indicates that the instanced struct is for cosmetic purposes only.
		CosmeticComponent,
	};

	enum
	{
		// [PropertyMetadata] Indicates that this property allows components that are marked as cosmetic.
		AllowCosmeticComponents,

		// [PropertyMetadata] Indicates that this property allows components that are not marked as cosmetic.
		AllowNonCosmeticComponents,
	};
}

/**
 * Item component data structure that can be extended to add additional data to an item.
 */
USTRUCT(BlueprintType, meta = (SingletonComponent = true, CosmeticComponent = false), DisplayName = "Item Component Data Item")
struct ITEMIZATIONCORERUNTIME_API FItemComponentData
{
	GENERATED_BODY()

public:
	virtual ~FItemComponentData() = default;

public:
	//~ Begin FItemComponentData Interface
	virtual void EvaluateContext(const FInventoryItemEntry& ItemEntry, FItemActionContextData& Context) const {}
	
	virtual bool CanCombineItemStacks(const FInventoryItemEntry& ThisEntry, const FInventoryItemEntry& OtherEntry, FItemActionContextData& Context) const { return true; }
	virtual bool CanCreateNewStack(const FInventoryItemEntry& ItemEntry, const FItemActionContextData& Context) const { return true; }
	virtual bool CanClearItem(const FInventoryItemEntry& ItemEntry) const { return true; }
	virtual bool IncludeInClearAll() const { return true; }
	
	virtual void OnItemStateChanged(const FInventoryItemEntryHandle& Handle, EUserFacingItemState NewState) const;
	virtual void OnItemInstanceCreated(const FInventoryItemEntryHandle& Handle, const FItemizationCoreInventoryData* InventoryData) const {}
	virtual void OnItemInstanceDestroyed(const FInventoryItemEntryHandle& Handle, const FItemizationCoreInventoryData* InventoryData) const {}
	//~ End FItemComponentData Interface

#if WITH_EDITOR
	virtual EDataValidationResult IsDataValid(class FDataValidationContext& Context) const;
#endif

protected:
	EUserFacingItemState CurrentState = EUserFacingItemState::Owned;
};

/**
 * Proxy struct for item component data that allows to identify components based on GUID in editor
 * (even empty ones).
 */
USTRUCT(BlueprintType, DisplayName = "Item Component Data")
struct ITEMIZATIONCORERUNTIME_API FItemComponentDataProxy
{
	GENERATED_BODY()

	FItemComponentDataProxy() = default;

	template<typename T, typename = std::enable_if_t<std::is_base_of_v<FItemComponentData, std::decay_t<T>>>>
	static FItemComponentDataProxy Make(const T& Struct)
	{
		FItemComponentDataProxy NewProxy;
		NewProxy.Component.InitializeAsScriptStruct(TBaseStructure<T>::Get(), reinterpret_cast<const uint8*>(&Struct));
#if WITH_EDITORONLY_DATA
		NewProxy.ComponentGuid = FGuid::NewGuid();
#endif
		return NewProxy;
	}

	UPROPERTY(EditDefaultsOnly, Category = "Component", meta = (ShowOnlyInnerProperties))
	TInstancedStruct<FItemComponentData> Component;

#if WITH_EDITORONLY_DATA
	UPROPERTY(EditDefaultsOnly, Category = "Component", meta = (EditConditionHides, EditCondition = false))
	FGuid ComponentGuid;
#endif
};
