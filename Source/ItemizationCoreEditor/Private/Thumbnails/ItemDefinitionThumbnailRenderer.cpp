// Fill out your copyright notice in the Description page of Project Settings.


#include "ItemDefinitionThumbnailRenderer.h"

#include "ItemDefinition.h"
#include "ItemizationUtilities.h"
#include "ObjectTools.h"

bool UItemDefinitionThumbnailRenderer::CanVisualizeAsset(UObject* Object)
{
	if (GetThumbnailTextureFromItem(Object) == nullptr)
	{
		return false;
	}

	return true;
}

void UItemDefinitionThumbnailRenderer::GetThumbnailSize(UObject* Object, float Zoom, uint32& OutWidth,
	uint32& OutHeight) const
{
	Super::GetThumbnailSize(GetThumbnailTextureFromItem(Object), Zoom, OutWidth, OutHeight);
}

void UItemDefinitionThumbnailRenderer::Draw(
	UObject* Object, int32 X, int32 Y, uint32 Width, uint32 Height,
	FRenderTarget* Viewport, FCanvas* Canvas, bool bAdditionalViewFamily)
{
	UTexture2D* ThumbnailTexture = GetThumbnailTextureFromItem(Object);
	if (ThumbnailTexture == nullptr)
	{
		ClearThumbnailCache(Object);
		return;
	}
	
	Super::Draw(ThumbnailTexture, X, Y, Width, Height, Viewport, Canvas, bAdditionalViewFamily);
}

void UItemDefinitionThumbnailRenderer::ClearThumbnailCache(UObject* Object)
{
	ThumbnailTools::CacheEmptyThumbnail(Object->GetFullName(), Object->GetOutermost());
}

UTexture2D* UItemDefinitionThumbnailRenderer::GetThumbnailTextureFromItem(UObject* Object) const
{
	UItemDefinition* ItemDefinition = Cast<UItemDefinition>(Object);
	if (ItemDefinition == nullptr)
	{
		return nullptr;
	}

	const FSoftObjectPath IconPath = UE::ItemizationCore::Display::FindItemIcon(ItemDefinition);
	if (IconPath.IsNull())
	{
		return nullptr;
	}

	return Cast<UTexture2D>(IconPath.TryLoad());
}
