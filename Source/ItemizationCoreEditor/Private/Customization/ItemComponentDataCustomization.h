// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"


class FItemComponentDataCustomization : public IPropertyTypeCustomization
{
public:
	using FThis = FItemComponentDataCustomization;
	static TSharedRef<IPropertyTypeCustomization> MakeInstance();

private:
	//~ Begin IPropertyTypeCustomization Interface
	virtual void CustomizeHeader(TSharedRef<IPropertyHandle> PropertyHandle, FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& CustomizationUtils) override;
	virtual void CustomizeChildren(TSharedRef<IPropertyHandle> PropertyHandle, IDetailChildrenBuilder& ChildBuilder, IPropertyTypeCustomizationUtils& CustomizationUtils) override;
	//~ End IPropertyTypeCustomization Interface

	TSharedRef<SWidget> GenerateStructPicker();
	void OnStructPicked(const UScriptStruct* InStruct);
	const FSlateBrush* GetDisplayValueIcon() const;
	EVisibility GetHiddenDataVisibility() const;
	FText GetDisplayIndexString() const;
	FText GetDisplayValueString() const;
	FText GetDisplayValueStringWrapped() const;
	FText GetTooltipText() const;

private:
	TSharedPtr<IPropertyHandle> ProxyProperty;
	TSharedPtr<IPropertyHandle> StructProperty;
	TSharedPtr<IPropertyHandleArray> ArrayProperty;
	TSharedPtr<SComboButton> ComboButton;
	TSharedPtr<SBorder> HitBox;
};
