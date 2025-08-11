// Author: Tom Werner (MajorT), 2025

#pragma once

#include "IPropertyTypeCustomization.h"

class SItemComponentDataPicker;

class FItemComponentDataCustomization : public IPropertyTypeCustomization
{
public:
	using ThisClass = FItemComponentDataCustomization;
	using Super = IPropertyTypeCustomization;

	static TSharedRef<IPropertyTypeCustomization> MakeInstance();
	virtual ~FItemComponentDataCustomization() override;

protected:
	//~ Begin IPropertyTypeCustomization Interface
	virtual void CustomizeHeader(TSharedRef<IPropertyHandle> PropertyHandle, FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& CustomizationUtils) override;
	virtual void CustomizeChildren(TSharedRef<IPropertyHandle> PropertyHandle, IDetailChildrenBuilder& ChildBuilder, IPropertyTypeCustomizationUtils& CustomizationUtils) override;
	//~ End IPropertyTypeCustomization Interface

	using FReplacementObjectMap = TMap<UObject*, UObject*>;
	void OnObjectsReInstanced(const FReplacementObjectMap& ObjectMap);

	FText GetDisplayValueText() const;
	FText GetTooltipText() const;

private:
	TSharedPtr<IPropertyHandle> InstanceProperty;
	TSharedPtr<IPropertyHandle> StructProperty;
	TSharedPtr<IPropertyHandleArray> ArrayProperty;

	TSharedPtr<IPropertyUtilities> PropUtils;
	TSharedPtr<SItemComponentDataPicker> StructPicker;

	FDelegateHandle OnObjectsReInstancedHandle;
};
