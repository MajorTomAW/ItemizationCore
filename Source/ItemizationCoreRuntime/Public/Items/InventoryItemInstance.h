// Author: Tom Werner (MajorT), 2025

#pragma once

#include "CoreMinimal.h"
#include "InventoryItemHandle.h"
#include "UObject/Object.h"

#include "Net/Core/PushModel/PushModelMacros.h"

#include "InventoryItemInstance.generated.h"

class AInventoryBase;
/**
 * 
 */
UCLASS(BlueprintType, Blueprintable)
class ITEMIZATIONCORERUNTIME_API UInventoryItemInstance : public UObject
{
	GENERATED_BODY()
	REPLICATED_BASE_CLASS(ThisClass)
	friend struct FInventoryItemEntry;
	friend struct FInventoryItemContainer;

public:
	UInventoryItemInstance(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	//~ Begin UObject Interface
	virtual UWorld* GetWorld() const override;
	virtual int32 GetFunctionCallspace(UFunction* Function, FFrame* Stack) override;
	virtual bool CallRemoteFunction(UFunction* Function, void* Parms, struct FOutParmRec* OutParms, FFrame* Stack) override;
	virtual bool IsSupportedForNetworking() const override;
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void PostInitProperties() override;

#if UE_WITH_IRIS
	virtual void RegisterReplicationFragments(UE::Net::FFragmentRegistrationContext& Context, UE::Net::EFragmentRegistrationFlags RegistrationFlags) override;
#endif

#if WITH_EDITOR
	virtual EDataValidationResult IsDataValid(class FDataValidationContext& Context) const override;
#endif
	//~ End UObject Interface

	/** Returns the source object that instigated the item instance creation. */
	UFUNCTION(BlueprintCallable, Category = Item)
	UObject* GetSourceObject() const;
	
	/** Template function to cast the source object to a specific type. */
	template <class T = UObject>
	T* GetSourceObject() const
	{
		return Cast<T>(GetSourceObject());
	}

	/** Gets the current item entry associated with the item handle of this instance. */
	FInventoryItemEntry* GetItemEntry() const;

	/** Returns whether replication is enabled or not. */
	bool GetIsReplicated() const { return bReplicates; }

	/** Gets the owning inventory for this item instance. Will fall back to the outer of this object. */
	UFUNCTION(BlueprintCallable, Category = Item)
	AInventoryBase* GetOwningInventory() const;

protected:
	/** If true, the item instance will replicate to remote machines. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Replication)
	uint8 bReplicates : 1;

	/** Cached reference to the inventory that this item instance is part of. This should usually be the same as the outer. */
	UPROPERTY(Transient)
	TWeakObjectPtr<AInventoryBase> OwningInventoryPtr;

	/** Handle to the item entry that this instance is associated with. */
	mutable FInventoryItemHandle ItemHandle;
};
