// Copyright © 2024 MajorT. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "Components/GameFrameworkComponent.h"

#include "InventoryExtensionComponent.generated.h"

struct FItemizationCoreInventoryData;
class APawn;
class AController;
class UInventoryManager;
struct FItemActionContextData;

/**
 * A generic component that can be used to extend the inventory system.
 * Examples provided by this plugin are the EquipmentManager and the InventorySlotManager.
 *
 * This component works hand-in-hand with the UInventoryManager and gets initialized by/with it.
 */
UCLASS(Abstract, ClassGroup=(Itemization))
class ITEMIZATIONCORERUNTIME_API UInventoryExtensionComponent : public UGameFrameworkComponent
{
	GENERATED_BODY()

public:
	UInventoryExtensionComponent(const FObjectInitializer& ObjectInitializer);

	/**
	 * Retrieves the Inventory Manager that owns the inventory.
	 * Will first try to resolve the weak reference, and if that fails from the inventory data and lastly from the owning actor.
	 */
	UInventoryManager* GetInventoryManager() const;
	template <class T>
	T* GetInventoryManager() const
	{
		static_assert(TPointerIsConvertibleFromTo<T, UInventoryManager>::Value, "'T' template parameter to 'GetInventoryManager' must be a subclass of UInventoryManager");
		return Cast<T>(GetInventoryManager());
	}

	/** Returns the inventory data. */
	FORCEINLINE const TSharedPtr<FItemizationCoreInventoryData>& GetInventoryData() const { return CachedInventoryData; }
	FORCEINLINE FItemizationCoreInventoryData* GetInventoryDataPtr() const { return CachedInventoryData.Get(); }

	/** If true, this component will try to initialize with the inventory data of the inventory manager. */
	uint8 bWantsInventoryData:1;

	/** Called to link up this inventory extension component with an inventory manager. */
	virtual void SetInventoryManager(UInventoryManager* InInventoryManager);

	/** Called right before this component is initialized with an inventory manager. */
	virtual void PreInitializeWithInventoryManager(UInventoryManager* InInventoryManager);

	/** Tries to initialize this component with the inventory manager. */
	virtual void TryInitializeWithInventoryManager();

	/** Returns true if this component's actor has authority. */
	virtual bool IsOwnerActorAuthoritative() const;

	/** Gets the pawn that owns the component, this should always be valid during gameplay but can return null in the editor */
	APawn* GetPawn() const;
	template <class T>
	T* GetPawn() const
	{
		static_assert(TPointerIsConvertibleFromTo<T, APawn>::Value, "'T' template parameter to 'GetPawn' must be a subclass of APawn");
		return Cast<T>(GetPawn());
	}
	template <class T>
	T* GetPawnChecked() const
	{
		static_assert(TPointerIsConvertibleFromTo<T, APawn>::Value, "'T' template parameter to 'GetPawnChecked' must be a subclass of APawn");
		return CastChecked<T>(GetPawn());
	}

	/** Gets the controller that owns this component, this will usually be null on simulated clients */
	AController* GetController() const;
	template <class T>
	T* GetController() const
	{
		static_assert(TPointerIsConvertibleFromTo<T, AController>::Value, "'T' template parameter to 'GetController' must be a subclass of AController");
		return Cast<T>(GetController());
	}

	/** Gets the avatar actor assigned to the inventory manager. */
	AActor* GetAvatarActor() const;
	template <class T>
	T* GetAvatarActor() const
	{
		static_assert(TPointerIsConvertibleFromTo<T, AActor>::Value, "'T' template parameter to 'GetAvatarActor' must be a subclass of AActor");
		return Cast<T>(GetAvatarActor());
	}

	/** Gets the owner actor assigned to the inventory manager. */
	AActor* GetOwnerActor() const;
	template <class T>
	T* GetOwnerActor() const
	{
		static_assert(TPointerIsConvertibleFromTo<T, AActor>::Value, "'T' template parameter to 'GetOwnerActor' must be a subclass of AActor");
		return Cast<T>(GetOwnerActor());
	}

public:
	//~ Begin UObject Interface
	virtual void PreNetReceive() override;
	virtual void BeginPlay() override;
	//~ End UObject Interface

	//~ Begin UActorComponent Interface
	virtual void OnRegister() override;
	virtual void InitializeComponent() override;
	virtual void UninitializeComponent() override;
	//~ End UActorComponent Interface

protected:
	/**
	 * Cached data about the equipment system such as the inventory manager, and so on.
	 * Utility-struct for easy access to those data.
	 *
	 * For simulated proxies this will be its own inventory data not linked to the inventory manager.
	 */
	TSharedPtr<FItemizationCoreInventoryData> CachedInventoryData;
};
