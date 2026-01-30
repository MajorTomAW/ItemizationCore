// Author: Tom Werner (MajorT), 2025

#pragma once

#include "CoreMinimal.h"
#include "IInventoryAbilityItemInstanceInterface.h"
#include "IInventoryItemInstanceInterface.h"
#include "InventoryHandle.h"
#include "InventoryItemId.h"
#include "UObject/Object.h"

#include "InventoryItemInstance.generated.h"

struct FInventoryItemEntry;
class AInventoryBase;
class UObject;
class AActor;
class UWorld;
class FLifetimeProperty;
class UFunction;
struct FFame;
struct FOutParmRec;

#define UE_API ITEMIZATIONCORE_API

/** Instance of an item in an inventory.
 * However, not every item entry has to have an instance!
 *
 * [NOTE] You don't necessarily need to subclass this UInventoryItemInstance, any object that inherits the
 * IInventoryItemInstanceInterface interface can act as one.
 */
UCLASS(BlueprintType, Blueprintable, MinimalAPI)
class UInventoryItemInstance
	: public UObject
	, public IInventoryItemInstanceInterface
	, public IInventoryAbilityItemInstanceInterface
{
	GENERATED_BODY()

public:
	UE_API UInventoryItemInstance(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	//~ Begin UObject Interface
	UE_API virtual UWorld* GetWorld() const override;
	UE_API virtual int32 GetFunctionCallspace(UFunction* Function, FFrame* Stack) override;
	UE_API virtual bool CallRemoteFunction(UFunction* Function, void* Parms, FOutParmRec* OutParms, FFrame* Stack) override;
	UE_API virtual bool IsSupportedForNetworking() const override;
	UE_API virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	UE_API virtual void PostInitProperties() override;

#if UE_WITH_IRIS
	UE_API virtual void RegisterReplicationFragments(UE::Net::FFragmentRegistrationContext& Context, UE::Net::EFragmentRegistrationFlags RegistrationFlags) override;
#endif

#if WITH_EDITOR
	UE_API virtual EDataValidationResult IsDataValid(FDataValidationContext& Context) const override;
#endif
	//~ End UObject Interface

	//~ Begin IInventoryItemInstanceInterface
	UE_API virtual void OnAddedToInventory(FInventoryItemEntry& ItemEntry, const FInventoryHandle& InventoryHandle) override;
	UE_API virtual void OnRemovedFromInventory(FInventoryItemEntry& ItemEntry, const FInventoryHandle& InventoryHandle) override;
	virtual inline bool GetIsReplicated() const override { return bReplicates; }
	UE_API virtual FInventoryItemEntry* GetItemEntry() const override;
	
	UFUNCTION(BlueprintCallable, Category = Item)
	UE_API virtual UObject* GetSourceObject() const override;
	//~ End IInventoryItemInstanceInterface

	//~ Begin IInventoryAbilityItemInstanceInterface
	virtual FGameplayAbilitySpecHandle TryGiveAbility(TSubclassOf<UGameplayAbility> AbilityClass, int32 Level = 0, int32 InputId = INDEX_NONE, FName SourceItemId = NAME_None) override;
	virtual void TryClearAbilities(FName SourceItemId) override;

	virtual FActiveGameplayEffectHandle TryApplyGameplayEffect(TSubclassOf<UGameplayEffect> EffectClass, float Level, FName SourceItemId = NAME_None) override;
	//~ End IInventoryAbilityItemInstanceInterface

	/** Returns the local role of the item's owner. */
	UE_API ENetRole GetLocalRole() const;

	/** Returns true if the item's owner has authority. */
	UE_API bool HasAuthority() const;

	/** Gets the owning inventory for this item instance. Will fall back to the outer of this object. */
	UFUNCTION(BlueprintCallable, Category = Item)
	UE_API AInventoryBase* GetOwningInventory() const;

protected:
	/** If true, the item instance will replicate to remote machines. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Replication)
	uint8 bReplicates : 1;

	/** Cached reference to the inventory that this item instance is part of. This should usually be the same as the outer. */
	UPROPERTY(Transient)
	FInventoryHandle OwningInventoryHandle;

	/** Authority-only handles to granted abilities and effects. */
	UPROPERTY(Transient)
	FAbilityItemGrantedHandlesContainer GrantedHandlesContainer;

	/** Item entry that this instance is associated with. */
	FInventoryItemEntry* ItemEntry;
};

#undef UE_API
