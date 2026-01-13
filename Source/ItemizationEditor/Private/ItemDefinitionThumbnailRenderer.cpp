// Author: Tom Werner (MajorT), 2025 November


#include "ItemDefinitionThumbnailRenderer.h"

#include "Items/ItemDefinitionBase.h"
#include "Items/Data/ItemComponentData_Icon.h"


#include UE_INLINE_GENERATED_CPP_BY_NAME(ItemDefinitionThumbnailRenderer)

void UItemDefinitionThumbnailRenderer::Draw(
	UObject* Object,
	int32 X, int32 Y,
	uint32 Width, uint32 Height,
	FRenderTarget* Viewport,
	FCanvas* Canvas,
	bool bAdditionalViewFamily)
{
	Super::Draw(GetThumbnailTexture(Object), X, Y, Width, Height, Viewport, Canvas, bAdditionalViewFamily);
}

bool UItemDefinitionThumbnailRenderer::CanVisualizeAsset(UObject* Object)
{
	return GetThumbnailTexture(Object) != nullptr;
}

UTexture2D* UItemDefinitionThumbnailRenderer::GetThumbnailTexture(UObject* Object) const
{
	if (UItemDefinitionBase* ItemDef = Cast<UItemDefinitionBase>(Object))
	{
		if (const FItemComponentData_Icon* IconData = ItemDef->GetItemData<FItemComponentData_Icon>())
		{
			if (UTexture2D* Texture = IconData->Icon.LoadSynchronous())
			{
				// we have to have completed build in order to get draw/get size
				Texture->BlockOnAnyAsyncBuild();
				return Texture;
			}
		}
	}

	return nullptr;
}
