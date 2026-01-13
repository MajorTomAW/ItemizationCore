// Author: Tom Werner (MajorT), 2025 November

#pragma once

#include "Styling/SlateStyle.h"

/** Implements the visual style of the itemization editor. */
class FItemizationEditorStyle final : public FSlateStyleSet
{
public:
	FItemizationEditorStyle();
	virtual ~FItemizationEditorStyle() override;

	static FItemizationEditorStyle& Get();

protected:
	friend class FItemizationEditorModule;

	static void Register();
	static void Unregister();

private:
	static TSharedPtr<FItemizationEditorStyle> Instance;
};
