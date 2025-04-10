// Copyright © 2025 MajorT. All Rights Reserved.

#pragma once

#include "ClassViewerFilter.h"

class FItemizationEditorAssetFilter : public IClassViewerFilter
{
public:
	FItemizationEditorAssetFilter()
		: DisallowedClassFlags(CLASS_Abstract | CLASS_Deprecated | CLASS_NewerVersionExists | CLASS_HideDropDown)
	{
	}

	explicit FItemizationEditorAssetFilter(const UClass* AllowedClass)
		: AllowedChildrenOfClasses({ AllowedClass })
		, DisallowedClassFlags(CLASS_Abstract | CLASS_Deprecated | CLASS_NewerVersionExists | CLASS_HideDropDown)
	{
	}
	
	TSet<const UClass*> AllowedChildrenOfClasses;
	EClassFlags DisallowedClassFlags;

protected:
	virtual bool IsClassAllowed(const FClassViewerInitializationOptions& InInitOptions, const UClass* InClass, TSharedRef<class FClassViewerFilterFuncs> InFilterFuncs) override
	{
		return InClass &&
			!InClass->HasAnyClassFlags(DisallowedClassFlags) &&
			InFilterFuncs->IfInChildOfClassesSet(AllowedChildrenOfClasses, InClass) != EFilterReturn::Failed;
	}

	virtual bool IsUnloadedClassAllowed(const FClassViewerInitializationOptions& InInitOptions, const TSharedRef<const class IUnloadedBlueprintData> InUnloadedClassData, TSharedRef<class FClassViewerFilterFuncs> InFilterFuncs) override
	{
		return !InUnloadedClassData->HasAnyClassFlags(DisallowedClassFlags) &&
			InFilterFuncs->IfInChildOfClassesSet(AllowedChildrenOfClasses, InUnloadedClassData) != EFilterReturn::Failed;
	}
};
