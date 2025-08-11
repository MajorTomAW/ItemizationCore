// Author: Tom Werner (MajorT), 2025

#pragma once

#include "CoreMinimal.h"
#include "ThumbnailRendering/ThumbnailRenderer.h"
#include "ItemDefinitionThumbnailRenderer.generated.h"

class UItemDefinitionBase;
/** Item Definition editor thumbnail renderer. */
UCLASS()
class ITEMIZATIONEDITOR_API UItemDefinitionThumbnailRenderer : public UThumbnailRenderer
{
	GENERATED_BODY()

protected:
	//~ Begin UThumbnailRenderer Interface
	virtual void GetThumbnailSize(UObject* Object, float Zoom, uint32& OutWidth, uint32& OutHeight) const override;
	virtual void Draw(UObject* Object, int32 X, int32 Y, uint32 Width, uint32 Height, FRenderTarget* Viewport, FCanvas* Canvas, bool bAdditionalViewFamily) override;
	virtual bool CanVisualizeAsset(UObject* Object) override;
	//~ End UThumbnailRenderer Interface

	UTexture2D* GetItemIcon(const UItemDefinitionBase* ItemDefinition) const;
};
