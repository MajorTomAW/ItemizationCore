// Author: Tom Werner (dc: majort), 2026 January

#pragma once

#include "CoreMinimal.h"
#include "GameplayAbilitySpecHandle.h"
#include "ActiveGameplayEffectHandle.h"
#include "Enums/EItemState.h"
#include "Enums/EItemStateFilter.h"
#include "UObject/Interface.h"

#include "IInventoryAbilityItemInstanceInterface.generated.h"

class UGameplayEffect;
struct FGameplayAbilitySpec;
class UGameplayAbility;
struct FActiveGameplayEffectHandle;
struct FGameplayAbilitySpecHandle;

USTRUCT()
struct FAbilityItemGrantedAbilityHandle
{
	GENERATED_BODY()

public:
	UPROPERTY()
	FGameplayAbilitySpecHandle AbilityHandle;

	UPROPERTY()
	EItemState StateWhenAdded = EItemState::InInventory;

	UPROPERTY()
	FName SourceId;
};

USTRUCT()
struct FAbilityItemGrantedEffectHandle
{
	GENERATED_BODY()

public:
	UPROPERTY()
	FActiveGameplayEffectHandle EffectHandle;

	UPROPERTY()
	EItemState StateWhenAdded = EItemState::InInventory;

	UPROPERTY()
	FName SourceId;
};

USTRUCT()
struct FAbilityItemGrantedHandlesContainer
{
	GENERATED_BODY()

public:
	void AddGrantedAbilityHandle(
		const FGameplayAbilitySpecHandle& GrantedHandle,
		EItemState StateWhenAdded,
		FName SourceId);

	void AppendGrantedHandlesContainer(
		const TArray<FGameplayAbilitySpecHandle>& GrantedHandles,
		EItemState StateWhenAdded,
		FName SourceId);

	void ClearGrantedAbilityHandles(
		EItemState FilterState,
		FName SourceId);



	void AddGrantedEffectHandle(
		const FActiveGameplayEffectHandle& GrantedHandle,
		EItemState StateWhenAdded,
		FName SourceId);

	void AppendGrantedEffectHandles(
		const TArray<FActiveGameplayEffectHandle>& GrantedHandles,
		EItemState StateWhenAdded,
		FName SourceId);

	void ClearGrantedEffectHandles(
		EItemState FilterState,
		FName SourceId);


	bool ContainsAbilityHandle(const FGameplayAbilitySpecHandle& Handle) const
	{
		return AbilityHandles.ContainsByPredicate([Handle] (const FAbilityItemGrantedAbilityHandle& Other)
		{
			return Other.AbilityHandle == Handle;
		});
	}

	bool ContainsEffectHandle(const FActiveGameplayEffectHandle& Handle) const
	{
		return EffectHandles.ContainsByPredicate([Handle] (const FAbilityItemGrantedEffectHandle& Other)
		{
			return Other.EffectHandle == Handle;
		});
	}

private:
	UPROPERTY(Transient)
	TArray<FAbilityItemGrantedAbilityHandle> AbilityHandles;

	UPROPERTY(Transient)
	TArray<FAbilityItemGrantedEffectHandle> EffectHandles;
};

inline void FAbilityItemGrantedHandlesContainer::AddGrantedAbilityHandle(
	const FGameplayAbilitySpecHandle& GrantedHandle,
	EItemState StateWhenAdded,
	FName SourceId)
{
	if (!GrantedHandle.IsValid())
	{
		return;
	}

	if (ContainsAbilityHandle(GrantedHandle))
	{
		return;
	}

	AbilityHandles.Add({GrantedHandle, StateWhenAdded, SourceId});
}

inline void FAbilityItemGrantedHandlesContainer::AppendGrantedHandlesContainer(
	const TArray<FGameplayAbilitySpecHandle>& GrantedHandles,
	EItemState StateWhenAdded,
	FName SourceId)
{
	for (const FGameplayAbilitySpecHandle& Handle : GrantedHandles)
	{
		if (!Handle.IsValid())
		{
			continue;
		}

		if (ContainsAbilityHandle(Handle))
		{
			continue;
		}

		AbilityHandles.Add({Handle, StateWhenAdded, SourceId});
	}
}

inline void FAbilityItemGrantedHandlesContainer::ClearGrantedAbilityHandles(
	EItemState FilterState,
	FName SourceId)
{
	for (auto It = AbilityHandles.CreateIterator(); It; ++It)
	{
		FAbilityItemGrantedAbilityHandle Granted = *It;
		if (Granted.StateWhenAdded == FilterState &&
			Granted.SourceId == SourceId)
		{
			It.RemoveCurrent();
		}
	}
}

inline void FAbilityItemGrantedHandlesContainer::AddGrantedEffectHandle(
	const FActiveGameplayEffectHandle& GrantedHandle,
	EItemState StateWhenAdded,
	FName SourceId)
{
	if (!GrantedHandle.IsValid())
	{
		return;
	}

	if (ContainsEffectHandle(GrantedHandle))
	{
		return;
	}

	EffectHandles.Add({GrantedHandle, StateWhenAdded, SourceId});
}

inline void FAbilityItemGrantedHandlesContainer::AppendGrantedEffectHandles(
	const TArray<FActiveGameplayEffectHandle>& GrantedHandles,
	EItemState StateWhenAdded,
	FName SourceId)
{
	for (const FActiveGameplayEffectHandle& GrantedHandle : GrantedHandles)
	{
		if (!GrantedHandle.IsValid())
		{
			return;
		}

		if (ContainsEffectHandle(GrantedHandle))
		{
			return;
		}

		EffectHandles.Add({GrantedHandle, StateWhenAdded, SourceId});
	}
}

inline void FAbilityItemGrantedHandlesContainer::ClearGrantedEffectHandles(
	EItemState FilterState,
	FName SourceId)
{
	for (auto It = EffectHandles.CreateIterator(); It; ++It)
	{
		FAbilityItemGrantedEffectHandle Granted = *It;
		if (Granted.StateWhenAdded == FilterState &&
			Granted.SourceId == SourceId)
		{
			It.RemoveCurrent();
		}
	}
}

/**
 * Interface for an object that can act as an ability item for an item entry in the inventory.
 * @note Your Item Instance still needs to implement the IInventoryItemInstanceInterface, as this one is just
 * an additional wrapper to enable granting / removing abilities and effects.
 */
UINTERFACE(meta=(CannotImplementInterfaceInBlueprint), MinimalAPI)
class UInventoryAbilityItemInstanceInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * Interface for an object that can act as an ability item for an item entry in the inventory.
 * @note Your Item Instance still needs to implement the IInventoryItemInstanceInterface, as this one is just
 * an additional wrapper to enable granting / removing abilities and effects.
 */
class IInventoryAbilityItemInstanceInterface
{
	GENERATED_BODY()

public:
	/** Attempts to give an ability to the item instances' owner. */
	virtual FGameplayAbilitySpecHandle TryGiveAbility(
		TSubclassOf<UGameplayAbility> AbilityClass,
		int32 Level = 0,
		int32 InputId = INDEX_NONE,
		FName SourceItemId = NAME_None) = 0;

	/** Attempts to remove all granted ability associated by SourceItemId. */
	virtual void TryClearAbilities(FName SourceItemId) = 0;

	/** Attempts to apply a gameplay effect to the item instances' owner. */
	virtual FActiveGameplayEffectHandle TryApplyGameplayEffect(
		TSubclassOf<UGameplayEffect> EffectClass,
		float Level,
		FName SourceItemId = NAME_None) = 0;
};
