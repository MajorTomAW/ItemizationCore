// Fill out your copyright notice in the Description page of Project Settings.


#include "ItemizationEditorStyle.h"

#include "Interfaces/IPluginManager.h"
#include "Styling/SlateStyleMacros.h"
#include "Styling/SlateStyleRegistry.h"

TSharedPtr<FItemizationEditorStyle> FItemizationEditorStyle::Singleton;

FItemizationEditorStyle::FItemizationEditorStyle()
	: FSlateStyleSet("ItemizationEditorStyle")
{
	// Load the style
	const FString BaseDir = IPluginManager::Get().FindPlugin(TEXT("ItemizationCore"))->GetBaseDir();
	FSlateStyleSet::SetContentRoot(FPaths::EngineContentDir() / TEXT("Editor/Slate"));
	FSlateStyleSet::SetCoreContentRoot(FPaths::EngineContentDir() / TEXT("Slate"));
	
	// Colors
	Set("Colors.ClassColor", FLinearColor(FColor::FromHex("#26601eff")));
	Set("Colors.TabColorScale", FLinearColor(FColor::FromHex("#26601eaa")));

	// Asset Icons
	Set("ItemComponentData_Icon", new IMAGE_BRUSH_SVG("Starship/AssetIcons/Texture2D_64", CoreStyleConstants::Icon20x20));
	
	// Editor Icons
	Set("Icons.Details", new IMAGE_BRUSH_SVG("Starship/Common/Details", CoreStyleConstants::Icon16x16));
	Set("Icons.Components", new IMAGE_BRUSH_SVG("Starship/Common/Component", CoreStyleConstants::Icon16x16));
	Set("Icons.Viewport", new IMAGE_BRUSH_SVG("Starship/Common/Viewports", CoreStyleConstants::Icon16x16));
	Set("Icons.DisplayInfo", new IMAGE_BRUSH_SVG("Starship/Common/SaveThumbnail", CoreStyleConstants::Icon16x16));
	Set("Icons.Placement", new IMAGE_BRUSH_SVG("Starship/Common/PlaceActors", CoreStyleConstants::Icon16x16));
	Set("Icons.FixUpAssetID", new IMAGE_BRUSH_SVG("Starship/Common/Adjust", CoreStyleConstants::Icon16x16));
	Set("Icons.Browse", new IMAGE_BRUSH_SVG("Starship/Common/ContentBrowser", CoreStyleConstants::Icon16x16));
	Set("Icons.Level", new IMAGE_BRUSH_SVG("Starship/AssetIcons/World_16", CoreStyleConstants::Icon16x16));
	Set("Icons.Reset", new IMAGE_BRUSH_SVG("Starship/Common/ResetToDefault", CoreStyleConstants::Icon16x16));
	Set("Icons.Clean", new IMAGE_BRUSH_SVG("Starship/GraphEditors/CleanUp", CoreStyleConstants::Icon16x16));
	Set("Icons.Equipment", new IMAGE_BRUSH_SVG("Starship/AssetIcons/Pawn_16", CoreStyleConstants::Icon16x16));

	FSlateStyleSet::SetContentRoot(FPaths::EngineContentDir() / TEXT("Slate"));
	Set("Icons.Development", new IMAGE_BRUSH_SVG("Starship/Common/settings", CoreStyleConstants::Icon16x16));


	FSlateStyleSet::SetContentRoot(FPaths::Combine(BaseDir, TEXT("Resources/Slate")));
	FSlateStyleSet::SetCoreContentRoot(FPaths::Combine(BaseDir, TEXT("Resources/Slate")));

	// Toolbar Icons
	Set("Icons.CreateItem", new IMAGE_BRUSH_SVG(TEXT("Icons/CreateItem"), CoreStyleConstants::Icon20x20));
	Set("Icons.CreateItem.Overlay", new IMAGE_BRUSH_SVG(TEXT("Icons/CreateItemOverlay"), CoreStyleConstants::Icon20x20, FStyleColors::AccentGreen));
	Set("Icons.CreateItem.Background", new IMAGE_BRUSH_SVG(TEXT("Icons/CreateItemBackground"), CoreStyleConstants::Icon20x20));

	FSlateStyleRegistry::RegisterSlateStyle(*this);
}

FItemizationEditorStyle::~FItemizationEditorStyle()
{
	FSlateStyleRegistry::UnRegisterSlateStyle(*this);
}

TSharedRef<FItemizationEditorStyle> FItemizationEditorStyle::Get()
{
	if (!Singleton.IsValid())
	{
		Singleton = MakeShared<FItemizationEditorStyle>();
	}

	return Singleton.ToSharedRef();
}
