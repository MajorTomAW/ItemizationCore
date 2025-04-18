// Copyright © 2025 MajorT. All Rights Reserved.


#include "Items/ItemDefinition.h"
#include "UObject/AssetRegistryTagsContext.h"

#if WITH_EDITOR
#include "UObject/ObjectSaveContext.h"
#endif

#include UE_INLINE_GENERATED_CPP_BY_NAME(ItemDefinition)

#define LOCTEXT_NAMESPACE "ItemDefinition"

UItemDefinition::UItemDefinition(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
#if WITH_EDITORONLY_DATA
	AssetType_Reg = AssetType.AssetTypeId;
#endif
}

FPrimaryAssetId UItemDefinition::GetPrimaryAssetId() const
{
	return FPrimaryAssetId(PrimaryAssetType.AssetTypeId, GetFName());
}

FText UItemDefinition::GetDisplayName() const
{
	if (ItemName.IsEmpty())
	{
		return FText::FromName(GetFName());
	}

	return ItemName;
}

FText UItemDefinition::GetItemDescription() const
{
	if (ItemDescription.IsEmpty())
	{
		return ItemShortDescription;
	}

	return ItemDescription;
}

FText UItemDefinition::GetItemShortDescription() const
{
	return ItemShortDescription;
}

FText UItemDefinition::GetItemRichDescription() const
{
	FText RawText = GetItemDescription();
	if (RawText.IsEmpty())
    {
        return FText::GetEmpty();
    }

	// @TODO: Add rich text formatting support
	// FText::Format(FText::FromString("<RichTextBlock>{0}</RichTextBlock>"), RawText);
	return RawText;
}

void UItemDefinition::GetItemComponent(
	EItemDataQueryResult& ExecResult,
	UScriptStruct* ComponentType,
	FItemComponentData& FoundComponent) const
{
	if (ComponentType == nullptr)
	{
		ExecResult = EItemDataQueryResult::NotFound;
		return;
	}

	// @TODO: Custom K2 Node for this function actually
}

const FItemComponentData* UItemDefinition::GetItemComponent(const UScriptStruct* PropertyType) const
{
	for (auto& ComponentInstance : ItemComponents)
	{
		if (ComponentInstance.IsOfType(PropertyType))
		{
			return ComponentInstance.GetComponent<FItemComponentData>();
		}
	}

	return nullptr;
}

TArray<const FItemComponentData*> UItemDefinition::GetAllItemComponents() const
{
	TArray<const FItemComponentData*> OutComponents;
	OutComponents.Reserve(ItemComponents.Num());

	for (const auto& Instance : ItemComponents)
	{
		if (const FItemComponentData* Component = Instance.GetComponent<FItemComponentData>())
		{
			OutComponents.Add(Component);
		}
	}

	return OutComponents;
}

void UItemDefinition::GetAssetRegistryTags(FAssetRegistryTagsContext Context) const
{
	Super::GetAssetRegistryTags(Context);

	for (const FItemComponentData* ComponentData : GetAllItemComponents())
	{
		if (ensure(ComponentData))
		{
			ComponentData->GetAssetRegistryTags(Context);
		}
	}
}

#if WITH_EDITOR
void UItemDefinition::PostLoad()
{
	Super::PostLoad();
	AssetType_Reg = AssetType.AssetTypeId;
}

void UItemDefinition::PostSaveRoot(FObjectPostSaveRootContext ObjectSaveContext)
{
	AssetType_Reg = AssetType.AssetTypeId;
}

EDataValidationResult UItemDefinition::IsDataValid(FDataValidationContext& Context) const
{
	EDataValidationResult Result = Super::IsDataValid(Context);

	for (const FItemComponentData* ComponentData : GetAllItemComponents())
	{
		if (ensure(ComponentData))
		{
			Result = CombineDataValidationResults(Result, ComponentData->IsDataValid(Context));
		}
	}

	return Result;
}
#endif
#undef LOCTEXT_NAMESPACE
