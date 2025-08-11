// Author: Tom Werner (MajorT), 2025


#include "ItemDefinitionThumbnailRenderer.h"

#include "CanvasItem.h"
#include "CanvasTypes.h"
#include "ObjectTools.h"
#include "Items/ItemDefinitionBase.h"
#include "Items/Data/ItemComponentData_Icon.h"
#include "ThumbnailRendering/ThumbnailManager.h"

void UItemDefinitionThumbnailRenderer::GetThumbnailSize(
	UObject* Object,
	float Zoom,
	uint32& OutWidth,
	uint32& OutHeight) const
{
	UItemDefinitionBase* ItemDefinition = Cast<UItemDefinitionBase>(Object);
	if (const UTexture2D* Texture  = GetItemIcon(ItemDefinition))
	{
		OutWidth = FMath::TruncToInt(Zoom * Texture->GetSurfaceWidth());
		OutHeight = FMath::TruncToInt(Zoom * Texture->GetSurfaceHeight());
	}
}

void UItemDefinitionThumbnailRenderer::Draw(
	UObject* Object,
	int32 X,
	int32 Y,
	uint32 Width,
	uint32 Height,
	FRenderTarget* Viewport,
	FCanvas* Canvas,
	bool bAdditionalViewFamily)
{
	UItemDefinitionBase* ItemDefinition = Cast<UItemDefinitionBase>(Object);
	UTexture2D* Texture2D = GetItemIcon(ItemDefinition);
	if (Texture2D != nullptr)
	{
		const bool bUseTranslucentBlend = Texture2D && Texture2D->HasAlphaChannel() && ((Texture2D->LODGroup == TEXTUREGROUP_UI) || (Texture2D->LODGroup == TEXTUREGROUP_Pixels2D));
		TRefCountPtr<FBatchedElementParameters> BatchedElementParameters;
		if (bUseTranslucentBlend)
		{
			// If using alpha, draw a checkerboard underneath first.
			const int32 CheckerDensity = 8;
			UTexture2D* Checker = UThumbnailManager::Get().CheckerboardTexture;
			Canvas->DrawTile(
				0.0f, 0.0f, Width, Height, // Dimensions
				0.0f, 0.0f, CheckerDensity, CheckerDensity, // UVs
				FLinearColor::White, Checker->GetResource()); // Tint & Texture
		}

		// Use A canvas tile item to draw
		FCanvasTileItem CanvasTile(FVector2D(X, Y), Texture2D->GetResource(), FVector2D(Width, Height), FLinearColor::White);

		// Alpha
		CanvasTile.BlendMode = bUseTranslucentBlend ? SE_BLEND_Translucent : SE_BLEND_Opaque;
		CanvasTile.BatchedElementParameters = BatchedElementParameters;
		CanvasTile.Draw(Canvas);

		if (Texture2D != nullptr)
		{
			/* Additional String Note Start */
			FString AdditionalNote = TEXT("");

			int32 AdditionalNoteWidth = 0;
			int32 AdditionalNoteHeight = 0;
			StringSize(GEngine->GetLargeFont(), AdditionalNoteWidth, AdditionalNoteHeight, AdditionalNote);

			float PaddingX = Width / 128.0f;
			float PaddingY = Height / 128.0f;
			float ScaleX = Width / 64.0f; //Text is 1/64'th of the size of the thumbnails
			float ScaleY = Height / 64.0f;

			FCanvasTextItem TextItem(FVector2D(Width - PaddingX - AdditionalNoteWidth * ScaleX, Height - PaddingY - AdditionalNoteHeight * ScaleY), FText::FromString(AdditionalNote), GEngine->GetLargeFont(), FLinearColor::White);
			TextItem.EnableShadow(FLinearColor::Black);
			TextItem.Scale = FVector2D(ScaleX, ScaleY);
			// Uncomment if you want the editor to write a text on our FortItem
			TextItem.Draw(Canvas);
			/* Additional String Note End */
		}
	}
}

bool UItemDefinitionThumbnailRenderer::CanVisualizeAsset(UObject* Object)
{
	if (Object->HasAnyFlags(RF_ClassDefaultObject))
	{
		return false; // CDOs should not be visualized
	}
	
	const UItemDefinitionBase* ItemDefinition = Cast<UItemDefinitionBase>(Object);
	if (GetItemIcon(ItemDefinition) != nullptr)
	{
		return true;
	}

	// Manually clear the thumbnail cache for this object
	ThumbnailTools::CacheEmptyThumbnail(Object->GetFullName(), Object->GetOutermost());
	return false;
}

UTexture2D* UItemDefinitionThumbnailRenderer::GetItemIcon(const UItemDefinitionBase* ItemDefinition) const
{
	if (!IsValid(ItemDefinition))
	{
		return nullptr;
	}

	if (const FItemComponentData_Icon* ItemComponentData =
		ItemDefinition->GetItemData<FItemComponentData_Icon>())
	{
		return ItemComponentData->Icon.LoadSynchronous();
	}

	return nullptr;
}
