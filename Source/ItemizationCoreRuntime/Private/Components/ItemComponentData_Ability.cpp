// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/ItemComponentData_Ability.h"

#include "AbilitySystemGlobals.h"
#include "ItemizationCoreLog.h"

#include "AbilitySystemComponent.h"
#include "ActorComponents/InventoryManager.h"
#include "Engine/AssetManager.h"
#include "Engine/StreamableManager.h"

FAbilityItemComponent_Ability::FAbilityItemComponent_Ability()
{
	ActiveState = EUserFacingItemState::Owned;
	DefaultLevel.Value = 1.f;
}

FItemComponentData_Ability::FItemComponentData_Ability()
{
}

// --> Owned
void FItemComponentData_Ability::OnItemInstanceCreated(
	const FInventoryItemEntryHandle& Handle, const FItemizationCoreInventoryData* InventoryData) const
{
	// Abilities should only be added on the server.
	if (!InventoryData->HasNetAuthority())
	{
		return;
	}
	
	const UInventoryManager* InventoryManager = InventoryData->InventoryManager.Get();
	UAbilitySystemComponent* ASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(InventoryData->AvatarActor.Get());

	if ((InventoryManager == nullptr) || (ASC == nullptr))
	{
		ITEMIZATION_LOG(Error, TEXT("Failed to get InventoryManager (%s) or AbilitySystemComponent (%s)."),
			*GetNameSafe(InventoryManager), *GetNameSafe(ASC));
		return;
	}

	FInventoryItemEntry* ItemEntry = InventoryManager->FindItemEntryFromHandle(Handle);
	if (ItemEntry == nullptr)
	{
		ITEMIZATION_LOG(Error, TEXT("Failed to find ItemEntry for Handle (%s)."), *Handle.ToString());
		return;
	}
	FItemizationGrantedHandles& GrantedHandles = ItemEntry->GrantedHandles;
	FStreamableManager& Streamable = UAssetManager::GetStreamableManager();
	
	for (const FAbilityItemComponent_Ability& Entry : AbilityList)
	{
		// Only owned abilities should be added.
		if (Entry.ActiveState != EUserFacingItemState::Owned)
		{
			continue;
		}

		if (Entry.Ability.IsNull())
		{
			continue;
		}
		
		UGameplayAbility* AbilityCDO = Streamable.LoadSynchronous<UClass>(Entry.Ability)->GetDefaultObject<UGameplayAbility>();
		FGameplayAbilitySpec AbilitySpec(AbilityCDO, Entry.DefaultLevel.Value);
		AbilitySpec.SourceObject = ItemEntry->Instance;
		AbilitySpec.DynamicAbilityTags.AddTag(Entry.InputTag);
		
		const FGameplayAbilitySpecHandle GrantedHandle = ASC->GiveAbility(AbilitySpec);
		GrantedHandles.AbilityHandles.Add(GrantedHandle);
	}
}

// --> Removed
void FItemComponentData_Ability::OnItemInstanceDestroyed(
	const FInventoryItemEntryHandle& Handle, const FItemizationCoreInventoryData* InventoryData) const
{
	// Abilities should only be removed on the server.
	if (!InventoryData->HasNetAuthority())
	{
		return;
	}
	
	const UInventoryManager* InventoryManager = InventoryData->InventoryManager.Get();
	UAbilitySystemComponent* ASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(InventoryData->AvatarActor.Get());

	if ((InventoryManager == nullptr) || (ASC == nullptr))
	{
		ITEMIZATION_LOG(Error, TEXT("Failed to get InventoryManager (%s) or AbilitySystemComponent (%s)."),
			*GetNameSafe(InventoryManager), *GetNameSafe(ASC));
		return;
	}

	FInventoryItemEntry* ItemEntry = InventoryManager->FindItemEntryFromHandle(Handle);
	if (ItemEntry == nullptr)
	{
		ITEMIZATION_LOG(Error, TEXT("Failed to find ItemEntry for Handle (%s)."), *Handle.ToString());
		return;
	}

	for (auto GrantedHandle : ItemEntry->GrantedHandles.AbilityHandles)
	{
		if (GrantedHandle.IsValid())
		{
			ASC->ClearAbility(GrantedHandle);
		}
	}

	ItemEntry->GrantedHandles.AbilityHandles.Reset();
}

// --> Pot. Equipped/Active
void FItemComponentData_Ability::OnItemStateChanged(
	const FInventoryItemEntryHandle& Handle, EUserFacingItemState NewState) const
{
	FItemComponentData::OnItemStateChanged(Handle, NewState);
}

void FItemComponentData_Ability::GrantAbilitiesDeferred()
{
}
