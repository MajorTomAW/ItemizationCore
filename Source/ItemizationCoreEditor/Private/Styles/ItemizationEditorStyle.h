// Fill out your copyright notice in the Description page of Project Settings.

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

private:
	static TSharedPtr<FItemizationEditorStyle> Singleton;
};