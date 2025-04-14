// Copyright © 2025 MajorT. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "StructUtils/InstancedStruct.h"
#include "UObject/Object.h"

#include "Inventory/InventoryBase.h"
#include "Inventory/InventoryProperties.h"

#include "InventorySetupDataBase.generated.h"

struct FFInventorySlotProcessor;
class ASlottableInventory;
class AEquippableInventory;
class AInventory;
class UItemDefinition;
enum class EItemizationInventoryCreationType : uint8;
class AInventoryBase;
class UObject;
class UInputAction;
class FString;
class FText;

/**
 * Data class that is used for setting up an inventory system.
 */
UCLASS(BlueprintType, Blueprintable, MinimalAPI, Const, Abstract)
class UInventorySetupDataBase : public UObject
{
	GENERATED_BODY()

public:
	UInventorySetupDataBase(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	/** 
	 * This function is called when the inventory is created.
	 * It can be used to spawn the inventory and set up any initial data.
	 * @param InOwner The owner of the inventory.
	 * @param CreationType The type of creation that is being used.
	 * @param OutRootInventory The first inventory that was created.
	 */
	virtual void SpawnInventory(AActor* InOwner, EItemizationInventoryCreationType CreationType, AInventoryBase*& OutRootInventory) {}
};

/**
 * Struct holding data about an item that should be given on startup or when the inventory gets created the first time.
 */
USTRUCT(BlueprintType)
struct FInventoryStartingItem
{
	GENERATED_BODY()
	
	FInventoryStartingItem()
		: ItemDefinition(nullptr)
		, EntityClassFilter(nullptr)
		, Amount(1)
	{
	}

public:
	/** The item definition to add to the inventory. */
	UPROPERTY(EditDefaultsOnly, Category = Inventory)
	TSoftObjectPtr<UItemDefinition> ItemDefinition;

	/** The entity class filter that will be used to determine if this item can be added to the inventory. */
	UPROPERTY(EditDefaultsOnly, Category = Inventory)
	TSoftClassPtr<UObject> EntityClassFilter;

	/** The amount of items to add to the inventory. */
	UPROPERTY(EditDefaultsOnly, Category = Inventory)
	uint32 Amount;
};

/**
 * The default setup data class for an inventory system.
 */
UCLASS(MinimalAPI)
class UInventorySetupDataBase_Default : public UInventorySetupDataBase
{
	GENERATED_BODY()

public:
	UInventorySetupDataBase_Default(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	//~ Begin UInventorySetupDataBase Interface
	virtual void SpawnInventory(AActor* InOwner, EItemizationInventoryCreationType CreationType, AInventoryBase*& OutRootInventory) override;
	//~ End UInventorySetupDataBase Interface

public:
	/** The default properties for the inventory. */
	UPROPERTY(EditDefaultsOnly, Category = Inventory, NoClear)
	TArray<TInstancedStruct<FInventoryProperties>> InventoryList;

	/** The default properties for an equippable inventory. */
	UPROPERTY(EditDefaultsOnly, Category = Inventory, NoClear)
	TArray<TInstancedStruct<FEquippableInventoryProperties>> EquippableInventoryList;

	/** The default slottable inventory properties. */
	UPROPERTY(EditDefaultsOnly, Category = Inventory, NoClear)
	const TInstancedStruct<FSlottableInventoryProperties> SlottableInventory;

	/** The list of starting items to add to the inventory. */
	UPROPERTY(EditDefaultsOnly, Category = Items, NoClear)
	TArray<TInstancedStruct<FInventoryStartingItem>> StartingItemList;

protected:
	template <typename PropertyType>
	void SpawnInventories(
		TArray<TInstancedStruct<PropertyType>> DataList,
		const FActorSpawnParameters& SpawnParams,
		TArray<AInventoryBase*>& InOutInventories);
};

template <typename PropertyType>
void UInventorySetupDataBase_Default::SpawnInventories(
	TArray<TInstancedStruct<PropertyType>> DataList,
	const FActorSpawnParameters& SpawnParams,
	TArray<AInventoryBase*>& InOutInventories)
{
	static_assert(TIsDerivedFrom<PropertyType, FInventoryPropertiesBase>::IsDerived,
		"PropertyType must be derived from FInventoryPropertiesBase");

	UWorld* const World = SpawnParams.Owner->GetWorld();
	check(World);

	for (const TInstancedStruct<PropertyType>& Data : DataList)
	{
		if (!ensure(Data.IsValid()))
		{
			continue;
		}

		const PropertyType* Prop = Data.template GetPtr<PropertyType>();
		if (!ensure(Prop))
		{
			continue;
		}

		TSoftClassPtr SoftClass = Prop->InventoryClass;
		if (!ensure(!SoftClass.IsNull()))
		{
			continue;
		}

		UClass* const Class = SoftClass.LoadSynchronous();
		AInventoryBase* Spawned = World->SpawnActor<AInventoryBase>(Class, SpawnParams);
		InOutInventories.Add(Spawned);
	}
}
