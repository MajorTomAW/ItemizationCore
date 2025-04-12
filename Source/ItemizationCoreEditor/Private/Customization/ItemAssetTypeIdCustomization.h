// Copyright © 2025 MajorT. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "IDetailCustomization.h"

class FItemAssetTypeIdCustomization : public IPropertyTypeCustomization
{
public:
	static TSharedRef<IPropertyTypeCustomization> MakeInstance();

	//~ Begin IDetailCustomization Interface
	virtual void CustomizeHeader(TSharedRef<IPropertyHandle> PropertyHandle, FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& CustomizationUtils) override;
	virtual void CustomizeChildren(TSharedRef<IPropertyHandle> PropertyHandle, IDetailChildrenBuilder& ChildBuilder, IPropertyTypeCustomizationUtils& CustomizationUtils) override;
	//~ End IDetailCustomization Interface

private:
	bool IsAssetTypeChangeable() const;
	TSharedRef<SWidget> GenerateAssetTypePicker();
	FText GetDisplayValueString() const;

	void OnAssetTypePicked(const FName& PickedType, TSharedPtr<IPropertyHandle> NameProperty);

private:
	TSharedPtr<IPropertyHandle> AssetTypeHandle;
	TSharedPtr<IPropertyHandle> AssetTypeIdHandle;
	TSharedPtr<SComboButton> ComboButton;
};
