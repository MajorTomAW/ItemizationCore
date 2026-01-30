// Author: Tom Werner (MajorT), 2025


#include "Items/ItemDefinitionBase.h"

#include "Items/InventoryItemInstance.h"

#if WITH_EDITOR
#include "UObject/ObjectSaveContext.h"
#include "Misc/DataValidation.h"
#endif

#include "ItemizationCoreLogChannels.h"
#include "Items/Data/ItemComponentData_Icon.h"
#include "Items/Data/ItemComponentData_MaxStackSize.h"
#include "Items/Data/ItemComponentData_Traits.h"
#include "UObject/AssetRegistryTagsContext.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(ItemDefinitionBase)

#define LOCTEXT_NAMESPACE "ItemDefinition"

UItemDefinitionBase::UItemDefinitionBase(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, ItemInstanceClass(UInventoryItemInstance::StaticClass())
{
	ItemAssetType = "ItemDefinition";
	ItemAssetId = GetFName();
}

FPrimaryAssetId UItemDefinitionBase::GetPrimaryAssetId() const
{
	return FPrimaryAssetId(
	ItemAssetType != NAME_None
			? ItemAssetType
			: TEXT("ItemDefinition"),
		ItemAssetId != NAME_None
			? ItemAssetId
			: GetFName());
}

void UItemDefinitionBase::GetAssetRegistryTags(FAssetRegistryTagsContext Context) const
{
	Super::GetAssetRegistryTags(Context);

	Context.AddTag(FAssetRegistryTag(
		FPrimaryAssetId::PrimaryAssetDisplayNameTag,
		ItemName.ToString(),
		FAssetRegistryTag::TT_Alphabetical));
}

#if WITH_EDITOR
void UItemDefinitionBase::PostSaveRoot(FObjectPostSaveRootContext ObjectSaveContext)
{
	ItemAssetId = GetFName();
	ItemId = GetPrimaryAssetId().ToString();
}

void UItemDefinitionBase::PostRename(UObject* OldOuter, const FName OldName)
{
	Super::PostRename(OldOuter, OldName);

	ItemAssetId = GetFName();
	ItemId = GetPrimaryAssetId().ToString();
	MarkPackageDirty();
}

void UItemDefinitionBase::PostLoad()
{
	Super::PostLoad();

	bool bShouldDirty = false;
	if (ItemAssetId != GetFName())
	{
		ItemAssetId = GetFName();
		bShouldDirty = true;
	}

	if (ItemId != GetPrimaryAssetId().ToString())
	{
		ItemId = GetPrimaryAssetId().ToString();
		bShouldDirty = true;
	}

	if (bShouldDirty)
	{
		MarkPackageDirty();
	}
}

EDataValidationResult UItemDefinitionBase::IsDataValid(class FDataValidationContext& Context) const
{
	EDataValidationResult Result = Super::IsDataValid(Context);

	if (ItemAssetType.IsNone() || !GetPrimaryAssetId().PrimaryAssetType.IsValid())
	{
		Result = EDataValidationResult::Invalid;
		Context.AddError(FText::Format(LOCTEXT("InvalidAssetType", "ItemDefinition '{0}' has an invalid PrimaryAssetType."),
			FText::FromName(GetFName())));
	}

	if (ItemAssetId.IsNone() || GetPrimaryAssetId().PrimaryAssetName.IsNone())
	{
		Result = EDataValidationResult::Invalid;
		Context.AddError(FText::Format(LOCTEXT("InvalidAssetName", "ItemDefinition '{0}' has an invalid PrimaryAssetName."),
			FText::FromName(GetFName())));
	}

	// Validate the item data
	for (const auto& Instance : DataList)
	{
		if (!Instance.Component.IsValid() || !Instance.Component.GetPtr<FItemComponentData>())
		{
			Result = EDataValidationResult::Invalid;
			Context.AddError(FText::Format(LOCTEXT("InvalidItemData", "ItemDefinition '{0}' has an invalid item component data."),
				FText::FromName(GetFName())));

			continue;
		}

		const FItemComponentData* Data = Instance.Component.GetPtr<FItemComponentData>();
		Result = CombineDataValidationResults(Result, Data->IsDataValid(Context));
	}

	return Result;
}
#endif

void UItemDefinitionBase::K2_QueryItemData(EItemDataQueryResult& Result, UScriptStruct* ItemDataType, int32& Value)
{
	// We should never hit this! stubs to avoid NoExport on the class.
	checkNoEntry();
}

DEFINE_FUNCTION(UItemDefinitionBase::execK2_QueryItemData)
{
	// Get the result enum (out ref)
	P_GET_ENUM_REF(EItemDataQueryResult, Result);

	// Get the item data type (in)
	P_GET_OBJECT_REF(UScriptStruct, ItemDataType);

	// Read wildcard value input
	Stack.MostRecentPropertyAddress = nullptr;
	Stack.MostRecentPropertyContainer = nullptr;
	Stack.StepCompiledIn<FStructProperty>(nullptr);

	const FStructProperty* ValueProp = CastField<FStructProperty>(Stack.MostRecentProperty);
	void* ValuePtr = Stack.MostRecentPropertyAddress;

	P_FINISH;

	// Set the result as "Not Found" for starters
	Result = EItemDataQueryResult::NotFound;

	P_NATIVE_BEGIN;
	for (const FItemComponentDataInstance* Data : P_THIS->GetDataListRaw())
	{
		if (Data->IsOfType(ItemDataType))
		{
			// Copy the memory to the Value out param
			ValueProp->Struct->CopyScriptStruct(ValuePtr, Data->Component.GetMemory());

			// Set the result to "Found"
			Result = EItemDataQueryResult::Found;

			break;
		}
	}
	P_NATIVE_END;
}

TArray<const FItemComponentData*> UItemDefinitionBase::GetDataList() const
{
	TArray<const FItemComponentData*> OutDataList;
	OutDataList.Reserve(DataList.Num());
	for (const auto& Instance : DataList)
	{
		if (const FItemComponentData* ItemData = Instance.Component.GetPtr<FItemComponentData>())
		{
			OutDataList.Add(ItemData);
		}
	}

	return OutDataList;
}

const FItemComponentData* UItemDefinitionBase::GetItemData(const UScriptStruct* PropertyType) const
{
	for (auto& Instance : DataList)
	{
		if (Instance.IsOfType(PropertyType))
		{
			return Instance.GetComponent<FItemComponentData>();
		}
	}

	return nullptr;
}


FText UItemDefinitionBase::GetItemName(bool bUsePlural) const
{
	if (ItemName.IsEmpty())
	{
		return FText::FromString(GetNameSafe(this));
	}

	return FText::Format(INVTEXT("{0}{1}|plural(other=s)"), ItemName, bUsePlural ? 2 : 1);
}

FText UItemDefinitionBase::GetItemTypeName() const
{
	return FText::FromString(GetPrimaryAssetId().PrimaryAssetType.ToString());
}

FText UItemDefinitionBase::GetItemDescription(bool bFallbackToShort) const
{
	if (!ItemDescription.IsEmpty())
	{
		return ItemDescription;
	}

	if (bFallbackToShort)
	{
		return GetItemShortDescription();
	}

	return FText::GetEmpty();
}

FText UItemDefinitionBase::GetItemRichDescription() const
{
	return ItemShortDescription;
}

bool UItemDefinitionBase::HasTrait(const FGameplayTag& TraitToCheck) const
{
	if (const FItemComponentData_Traits* TraitsData = GetItemData<FItemComponentData_Traits>())
	{
		return TraitsData->HasTrait(TraitToCheck);
	}

	return false;
}

int32 UItemDefinitionBase::GetMaxStackSize() const
{
	if (const FItemComponentData_MaxStackSize* MaxStackSizeData = GetItemData<FItemComponentData_MaxStackSize>())
	{
		return MaxStackSizeData->GetMaxStackSize();
	}

	// Without max stack size item data, max stack size is assumed to be 1
	return 1;
}

TArray<TSoftObjectPtr<const UScriptStruct>> UItemDefinitionBase::GetDisallowedDataTypes() const
{
	// By default, disallow structs that are already present
	TArray<TSoftObjectPtr<const UScriptStruct>> DisallowedTypes;

	for (const FItemComponentDataInstance& Instance : DataList)
	{
		if (!Instance.Component.IsValid())
		{
			continue;
		}

		DisallowedTypes.Add(Instance.Component.GetScriptStruct());
	}

	return DisallowedTypes;
}

#undef LOCTEXT_NAMESPACE
