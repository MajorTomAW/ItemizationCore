// Author: Tom Werner (MajorT), 2025


#include "Items/ItemDefinitionBase.h"

#include "Items/Data/ItemComponentData.h"

UItemDefinitionBase::UItemDefinitionBase()
{
}

TArray<const FItemComponentData*> UItemDefinitionBase::GetDataList() const
{
	TArray<const FItemComponentData*> OutDataList;
	OutDataList.Reserve(DataList.Num());
	for (const auto& Instance : DataList)
	{
		if (const FItemComponentData* ItemData = Instance.GetComponent<FItemComponentData>())
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
