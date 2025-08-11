// Author: Tom Werner (MajorT), 2025

#pragma once

#include "Styling/SlateStyle.h"

/**
 * Implements the visual style of the ItemizationCore editor.
 */
class FItemizationEditorStyle final : public FSlateStyleSet
{
public:
	FItemizationEditorStyle();
	virtual ~FItemizationEditorStyle() override;

	static TSharedRef<FItemizationEditorStyle> Get();
	static void Shutdown();

private:
	static TSharedPtr<FItemizationEditorStyle> Singleton;
};