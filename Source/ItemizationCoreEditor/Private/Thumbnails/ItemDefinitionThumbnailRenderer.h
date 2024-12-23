// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ThumbnailRendering/TextureThumbnailRenderer.h"
#include "ItemDefinitionThumbnailRenderer.generated.h"


UCLASS()
class UItemDefinitionThumbnailRenderer : public UTextureThumbnailRenderer
{
	GENERATED_BODY()

protected:
	//~ Begin UThumbnailRenderer Interface
	virtual bool CanVisualizeAsset(UObject* Object) override;
	virtual void GetThumbnailSize(UObject* Object, float Zoom, uint32& OutWidth, uint32& OutHeight) const override;
	virtual void Draw(UObject* Object, int32 X, int32 Y, uint32 Width, uint32 Height, FRenderTarget* Viewport, FCanvas* Canvas, bool bAdditionalViewFamily) override;
	//~ End UThumbnailRenderer Interface
	
private:
	void ClearThumbnailCache(UObject* Object);
	UTexture2D* GetThumbnailTextureFromItem(UObject* Object) const;
};
