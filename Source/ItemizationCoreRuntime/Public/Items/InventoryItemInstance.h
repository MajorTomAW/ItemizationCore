// Copyright © 2025 MajorT. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Net/Core/PushModel/PushModelMacros.h"
#include "UObject/Object.h"

#include "InventoryItemInstance.generated.h"

class FObjectInitializer;
class FLifetimeProperty;
class UWorld;
class UFunction;
class UObject;
class AActor;

/**
 * Instance of an item in an inventory.
 * Can be used to define custom gameplay logic.
 */
UCLASS(Blueprintable)
class ITEMIZATIONCORERUNTIME_API UInventoryItemInstance
	: public UObject
{
	GENERATED_BODY()
	REPLICATED_BASE_CLASS(UInventoryItemInstance)

public:
	UInventoryItemInstance(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	//~ Begin UObject Interface
	virtual UWorld* GetWorld() const override;
	virtual int32 GetFunctionCallspace(UFunction* Function, FFrame* Stack) override;
	virtual bool CallRemoteFunction(UFunction* Function, void* Parameters, FOutParmRec* OutParms, FFrame* Stack) override;
	virtual bool IsSupportedForNetworking() const override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

#if UE_WITH_IRIS
	virtual void RegisterReplicationFragments(UE::Net::FFragmentRegistrationContext& Context, UE::Net::EFragmentRegistrationFlags RegistrationFlags) override;
#endif

#if WITH_EDITOR
	virtual EDataValidationResult IsDataValid(FDataValidationContext& Context) const override;
#endif
	//~ End UObject Interface

public:
	/** Returns whether replication is enabled or not. */
	FORCEINLINE bool GetIsReplicated() const { return bReplicates; }

protected:
	/** If true, the item instance will replicate to remote machines. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Replication)
	uint8 bReplicates : 1;
};
