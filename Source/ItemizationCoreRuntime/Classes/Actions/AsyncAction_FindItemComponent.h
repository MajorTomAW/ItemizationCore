// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/CancellableAsyncAction.h"
#include "AsyncAction_FindItemComponent.generated.h"

class UItemDefinition;

/**
 * 
 */
UCLASS(BlueprintType, meta = (HasDedicatedAsyncNode))
class ITEMIZATIONCORERUNTIME_API UAsyncAction_FindItemComponent : public UCancellableAsyncAction
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Itemization Core", meta = (BlueprintInternalUseOnly = "true"))
	static UAsyncAction_FindItemComponent* FindItemComponent(UItemDefinition* ItemDefinition, UScriptStruct* ComponentType);

	UFUNCTION(BlueprintCallable, CustomThunk, Category = "Itemization Core", meta = (CustomStructureParam = "OutComponent"))
	bool GetComponent(UPARAM(ref) int32& OutComponent);

	DECLARE_FUNCTION(execGetComponent);

	virtual void Activate() override;
	virtual void SetReadyToDestroy() override;

private:
	const void* FoundItemComponentPtr = nullptr;
	
	TWeakObjectPtr<UWorld> WorldPtr;
	TWeakObjectPtr<UItemDefinition> ItemPtr;
	TWeakObjectPtr<UScriptStruct> ComponentTypePtr;
};
