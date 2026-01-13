// Author: Tom Werner (MajorT), 2025 November


#include "ItemizationEditorStyle.h"

#include "Interfaces/IPluginManager.h"
#include "Styling/SlateStyleRegistry.h"
#include "Styling/SlateStyleMacros.h"

TSharedPtr<FItemizationEditorStyle> FItemizationEditorStyle::Instance = nullptr;

FItemizationEditorStyle::FItemizationEditorStyle()
	: FSlateStyleSet("ItemizationEditorStyle")
{
	// Load the style
	const FString BaseDir = IPluginManager::Get().FindPlugin(TEXT("ItemizationCore"))->GetBaseDir();
	FSlateStyleSet::SetContentRoot(FPaths::EngineContentDir() / TEXT("Editor/Slate"));
	FSlateStyleSet::SetCoreContentRoot(FPaths::EngineContentDir() / TEXT("Slate"));

	const FScrollBarStyle ScrollBar = FAppStyle::GetWidgetStyle<FScrollBarStyle>("ScrollBar");
	const FTextBlockStyle& NormalText = FAppStyle::Get().GetWidgetStyle<FTextBlockStyle>("NormalText");

	// Item Data
	{
		Set("ItemComponent.Data.Border", new FSlateRoundedBoxBrush(FLinearColor::White, 6.f));
		Set("ItemComponent.Data", new FSlateRoundedBoxBrush(FLinearColor::White, 4.f));
	}

	// Widgets
	const FLinearColor SelectionColor = FColor(0, 0, 0, 32);
	const FTableRowStyle& NormalTableRowStyle = FAppStyle::Get().GetWidgetStyle<FTableRowStyle>("TableView.Row");
	Set("ItemData.Selection",
		FTableRowStyle(NormalTableRowStyle)
		.SetActiveBrush(CORE_IMAGE_BRUSH("Common/Selection", CoreStyleConstants::Icon8x8, SelectionColor))
		.SetActiveHoveredBrush(CORE_IMAGE_BRUSH("Common/Selection", CoreStyleConstants::Icon8x8, SelectionColor))
		.SetInactiveBrush(CORE_IMAGE_BRUSH("Common/Selection", CoreStyleConstants::Icon8x8, SelectionColor))
		.SetInactiveHoveredBrush(CORE_IMAGE_BRUSH("Common/Selection", CoreStyleConstants::Icon8x8, SelectionColor))
		.SetSelectorFocusedBrush(CORE_IMAGE_BRUSH("Common/Selection", CoreStyleConstants::Icon8x8, SelectionColor))
	);

	// Text
	{
		const FLinearColor ForegroundCol =  FStyleColors::Foreground.GetSpecifiedColor();

		Set("ItemData.Description", FTextBlockStyle(NormalText)
			.SetFont(DEFAULT_FONT("Regular", 10))
			.SetColorAndOpacity(ForegroundCol.CopyWithNewOpacity(0.8f)));

		Set("ItemData.Title", FTextBlockStyle(NormalText)
			.SetFont(DEFAULT_FONT("Bold", 12))
			.SetColorAndOpacity(FLinearColor(230.0f / 255.0f, 230.0f / 255.0f, 230.0f / 255.0f, 0.9f)));
	}

	// Colors
	Set("Colors.ItemDefinitionBase", FLinearColor(FColor::FromHex("#26601eff")));
	Set("Colors.TabColorScale", FLinearColor(FColor::FromHex("#26601eaa")));

	// Editor Icons
	Set("Icons.Details", new IMAGE_BRUSH_SVG("Starship/Common/Details", CoreStyleConstants::Icon16x16));
	Set("Icons.DataList", new IMAGE_BRUSH_SVG("Starship/Common/Component", CoreStyleConstants::Icon16x16));
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
	Set("Icons.Settings", new IMAGE_BRUSH_SVG("Starship/Common/settings", CoreStyleConstants::Icon16x16));
}

FItemizationEditorStyle::~FItemizationEditorStyle()
{
	Unregister();
}

FItemizationEditorStyle& FItemizationEditorStyle::Get()
{
	if (!Instance.IsValid())
	{
		Instance = MakeShared<FItemizationEditorStyle>();
	}

	return *Instance.Get();
}

void FItemizationEditorStyle::Register()
{
	FSlateStyleRegistry::RegisterSlateStyle(Get());
}

void FItemizationEditorStyle::Unregister()
{
	if (Instance.IsValid())
	{
		FSlateStyleRegistry::UnRegisterSlateStyle(*Instance);
		Instance.Reset();
	}
}
