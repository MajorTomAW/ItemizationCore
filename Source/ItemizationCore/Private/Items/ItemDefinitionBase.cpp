// Author: Tom Werner (MajorT), 2025


#include "Items/ItemDefinitionBase.h"

#include "Items/InventoryItemInstance.h"

#if WITH_EDITOR
#include "UObject/ObjectSaveContext.h"
#endif

#include "ItemizationCoreLogChannels.h"
#include "Items/Data/ItemComponentData_Icon.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(ItemDefinitionBase)

UItemDefinitionBase::UItemDefinitionBase(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, bWantsItemInstance(true) // Default to true
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

#if WITH_EDITOR
void UItemDefinitionBase::PostSaveRoot(FObjectPostSaveRootContext ObjectSaveContext)
{
	ItemAssetId = GetFName();
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
	//@TODO: Implement rich description logic, possibly using a data table or similar structure.
	return FText::Format(
		NSLOCTEXT("ItemDefinitionBase", "RichDescriptionFormat", "{0}\n\n{1}"),
		ItemShortDescription,
		ItemDescription);
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
