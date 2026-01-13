// Author: Tom Werner (MajorT), 2025 November

#pragma once

#include "CoreMinimal.h"
#include "ThumbnailRendering/TextureThumbnailRenderer.h"

#include "ItemDefinitionThumbnailRenderer.generated.h"

UCLASS()
class UItemDefinitionThumbnailRenderer : public UTextureThumbnailRenderer
{
	GENERATED_BODY()

protected:
	virtual void Draw(UObject* Object, int32 X, int32 Y, uint32 Width, uint32 Height, FRenderTarget* Viewport, FCanvas* Canvas, bool bAdditionalViewFamily) override;
	virtual bool CanVisualizeAsset(UObject* Object) override;

protected:
	virtual UTexture2D* GetThumbnailTexture(UObject* Object) const;
};
