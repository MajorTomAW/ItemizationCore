// Copyright © 2025 MajorT. All Rights Reserved.


#include "Items/InventoryItemInstance.h"

#if UE_WITH_IRIS
#include "Iris/ReplicationSystem/ReplicationFragmentUtil.h"
#endif

#if WITH_EDITOR
#include "Misc/DataValidation.h"
#endif

#include "ItemizationLogChannels.h"
#include "Inventory/Inventory.h"
#include "Net/UnrealNetwork.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(InventoryItemInstance)

#define LOCTEXT_NAMESPACE "InventoryItemInstance"

UInventoryItemInstance::UInventoryItemInstance(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bReplicates = false;
	ItemDefinition = nullptr;
}

UWorld* UInventoryItemInstance::GetWorld() const
{
	if (HasAnyFlags(RF_ClassDefaultObject))
	{
		// If we're CDO, we must return nullptr instead of calling Outer->GetWorld() to fool UObject::ImplementsGetWorld().
		ITEMIZATION_ERROR("GetWorld() called on CDO of %s", *GetName());
		return nullptr;
	}
	
	return GetOuter()->GetWorld();
}

int32 UInventoryItemInstance::GetFunctionCallspace(
	UFunction* Function,
	FFrame* Stack)
{
	if (HasAnyFlags(RF_ClassDefaultObject) || !IsSupportedForNetworking())
	{
		// This handles absorbing authority/cosmetic.
		GEngine->GetGlobalFunctionCallspace(Function, this, Stack);
	}

	UObject* Outer = GetOuter();
	check(Outer);
	return Outer->GetFunctionCallspace(Function, Stack);
}

bool UInventoryItemInstance::CallRemoteFunction(
	UFunction* Function,
	void* Parameters,
	FOutParmRec* OutParms,
	FFrame* Stack)
{
	// No need to execute RPCs if we're pending kill
	if (!IsValid(this))
	{
		return false;
	}

	check(!HasAnyFlags(RF_ClassDefaultObject));
	check(GetOuter() != nullptr);

	AActor* Owner = CastChecked<AActor>(GetOuter());
	bool bProcessed = false;

	FWorldContext* const Context = GEngine->GetWorldContextFromWorld(GetWorld());
	if (Context != nullptr)
	{
		for (FNamedNetDriver& Driver : Context->ActiveNetDrivers)
		{
			if ((Driver.NetDriver != nullptr) &&
				(Driver.NetDriver->ShouldReplicateFunction(Owner, Function)))
			{
				Driver.NetDriver->ProcessRemoteFunction(Owner, Function, Parameters, OutParms, Stack, this);
				bProcessed = true;
			}
		}
	}

	return bProcessed;
}

bool UInventoryItemInstance::IsSupportedForNetworking() const
{
	return GetOuter()->IsA(UPackage::StaticClass()) || GetIsReplicated();
}

void UInventoryItemInstance::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	UObject::GetLifetimeReplicatedProps(OutLifetimeProps);

	if (UBlueprintGeneratedClass* BPClass = Cast<UBlueprintGeneratedClass>(GetClass()))
	{
		BPClass->GetLifetimeBlueprintReplicationList(OutLifetimeProps);
	}

	FDoRepLifetimeParams SharedParams;
	SharedParams.bIsPushBased = true;
}

void UInventoryItemInstance::PostInitProperties()
{
	UObject::PostInitProperties();
}

AInventory* UInventoryItemInstance::GetOwningInventory() const
{
	if (OwningInventoryPtr.IsValid())
	{
		return OwningInventoryPtr.Get();
	}

	if (AInventory* OuterAsInventory = Cast<AInventory>(GetOuter()))
	{
		return OuterAsInventory;
	}

	return nullptr;
}

UItemDefinition* UInventoryItemInstance::GetItemDefinition_Typed(TSubclassOf<UItemDefinition> ItemClass) const
{
	UItemDefinition* Definition = GetItemDefinition();
	if (Definition && Definition->IsA(ItemClass))
	{
		return Definition;
	}

	return nullptr;
}

FInventoryItemEntry* UInventoryItemInstance::GetItemEntry() const
{
	AInventory const* MyInventory = GetOwningInventory();
	check(MyInventory);

	return MyInventory->FindItemEntryFromHandle(ItemHandle);
}

UObject* UInventoryItemInstance::GetSourceObject() const
{
	if (const FInventoryItemEntry* ItemEntry = GetItemEntry())
	{
		return ItemEntry->SourceObject.Get();
	}

	return nullptr;
}

#if UE_WITH_IRIS
void UInventoryItemInstance::RegisterReplicationFragments(
	UE::Net::FFragmentRegistrationContext& Context,
	UE::Net::EFragmentRegistrationFlags RegistrationFlags)
{
	// Build description and allocate PropertyReplicationFragments for this UObject.
	UE::Net::FReplicationFragmentUtil::CreateAndRegisterFragmentsForObject(this, Context, RegistrationFlags);
}
#endif
#if WITH_EDITOR
EDataValidationResult UInventoryItemInstance::IsDataValid(FDataValidationContext& Context) const
{
	EDataValidationResult Result = EDataValidationResult::Valid;
	const bool bIsLikelyRunningAutomation = IsRunningCommandlet() || FApp::IsUnattended();

	if (bReplicates)
	{
		const UClass* Class = GetClass();
		for (const FProperty* Property = Class->PropertyLink; Property; Property = Property->PropertyLinkNext)
		{
			if (Property->RepIndex > 0 || Property->HasAnyPropertyFlags(CPF_RepNotify | CPF_Net))
			{
				Context.AddWarning(FText::Format(LOCTEXT("ReplicatedPropertiesAreDeprecated",
					"{0}: Replicated properties are deprecated in Inventory Items. Use Reliable RPCs for variable replication."),
					Property->GetDisplayNameText()));

				Result = bIsLikelyRunningAutomation
					? CombineDataValidationResults(Result, EDataValidationResult::Valid)
					: EDataValidationResult::Invalid;
			}
		}
	}
	else
	{	UBlueprintGeneratedClass* BPClass = Cast<UBlueprintGeneratedClass>(GetClass());
		if (BPClass && BPClass->NumReplicatedProperties > 0)
		{
			Context.AddError(LOCTEXT("ReplicatedPropertiesNeedReplication", "Item Instance Blueprint has replicated variables but bReplicates is set to false"));
			Result =  EDataValidationResult::Invalid;
		}
	}

	for (TFieldIterator<const UFunction> FuncIt(GetClass(), EFieldIterationFlags::IncludeSuper); FuncIt; ++FuncIt)
	{
		if (FuncIt->HasAnyFunctionFlags(FUNC_NetMulticast))
		{
			FText ErrorText = FText::Format(LOCTEXT("MulticastFunctionDisallowed",
				"Inventory Items are not replicated to Simulated Proxies and therefore NetMulticast Function {0} is meaningless"),
				FText::FromString(FuncIt->GetName()));

			Context.AddError(ErrorText);
			Result = EDataValidationResult::Invalid;
		}

		if (FuncIt->HasAnyFunctionFlags(FUNC_Net) && !bReplicates)
		{
			FText ErrorText = FText::Format(LOCTEXT("RpcRequiresReplicationYes",
				"{0}: RPC Functions require bReplicated to be true in order to actually work."),
				FuncIt->GetDisplayNameText());

			if (bIsLikelyRunningAutomation)
			{
				Context.AddWarning(ErrorText);
				Result = CombineDataValidationResults(Result, EDataValidationResult::Valid);
			}
			else
			{
				Context.AddError(ErrorText);
				Result = EDataValidationResult::Invalid;
			}
		}
	}

	return Result;
}
#endif

void UInventoryItemInstance::GetOwnedGameplayTags(FGameplayTagContainer& TagContainer) const
{
	if (const FInventoryItemEntry* ItemEntry = GetItemEntry())
	{
		TagContainer.AppendTags(ItemEntry->DynamicTags);
	}
}

void UInventoryItemInstance::PostNetInit() const
{
}

void UInventoryItemInstance::OnAddedToInventory(FInventoryItemEntry& ItemEntry, AInventory* InOwningInventory)
{
	if (!ensureMsgf(!HasAnyFlags(RF_ClassDefaultObject),
		TEXT("%s must be instantiated because class default objects are not supported."), *GetName()))
	{
		return;
	}

	// Cache the item handle
	ItemHandle = ItemEntry.ItemHandle;

	// Notify the item component datas about the creating of this instance
	for (const FItemComponentData* ComponentData : ItemEntry.Definition->GetAllItemComponents())
	{
		if (ensure(ComponentData))
		{
			//ComponentData->OnItemInstanceCreated(ItemEntry, this); @TODO: Create OnItemInstanceCreated() function
		}
	}

	// Make sure our owner actor is valid
}

void UInventoryItemInstance::OnRemovedFromInventory(FInventoryItemEntry& ItemEntry, AInventory* InOwningInventory)
{
}

#undef LOCTEXT_NAMESPACE
