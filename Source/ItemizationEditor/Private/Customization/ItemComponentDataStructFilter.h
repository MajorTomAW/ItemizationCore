// Author: MajorT (Tom Werner), 2025

#pragma once

#include "StructViewerFilter.h"

#include "StructUtils/UserDefinedStruct.h"

class FItemComponentDataStructFilter : public IStructViewerFilter
{
public:
	/** optimal outer class for the filter */
	const UClass* OuterClass = nullptr;
	
	/** The base struct for the property that classes must be a child-of. */
	TWeakObjectPtr<const UScriptStruct> BaseStruct = nullptr;

	/** Flag controlling whether we allow to select the BaseStruct. */
	bool bAllowBaseStruct = true;

	// A flag controlling whether we allow UserDefinedStructs
	bool bAllowUserDefinedStructs = false;

	/** The array of allowed structs */
	TArray<TSoftObjectPtr<const UScriptStruct>> AllowedStructs;

	/** The array of disallowed structs */
	TArray<TSoftObjectPtr<const UScriptStruct>> DisallowedStructs;

	// Optional filter to prevent selection of some structs e.g. ones in a plugin that is inaccessible from the object being edited
	TSharedPtr<IAssetReferenceFilter> AssetReferenceFilter;

public:
	virtual bool IsStructAllowed(const FStructViewerInitializationOptions& InInitOptions, const UScriptStruct* InStruct, TSharedRef<class FStructViewerFilterFuncs> InFilterFuncs) override
	{
		bool bStructAllowed = true;
		if (!AllowedStructs.IsEmpty())
		{
			bStructAllowed = false;
			for (const TSoftObjectPtr<const UScriptStruct>& AllowedStruct : AllowedStructs)
			{
				if (InStruct->IsChildOf(AllowedStruct.Get()))
				{
					bStructAllowed = true;
					break;
				}
			}
		}
		if (!DisallowedStructs.IsEmpty())
		{
			for (const TSoftObjectPtr<const UScriptStruct>& DisallowedStruct : DisallowedStructs)
			{
				if (InStruct->IsChildOf(DisallowedStruct.Get()))
				{
					bStructAllowed = false;
					break;
				}
			}
		}

		if (!bStructAllowed)
		{
			return false;
		}
	
		if (InStruct->IsA<UUserDefinedStruct>())
		{
			return bAllowUserDefinedStructs;
		}

		if (InStruct == BaseStruct)
		{
			return bAllowBaseStruct;
		}

		static const FName NAME_HiddenMetaTag = "Hidden";
		if (InStruct->HasMetaData(NAME_HiddenMetaTag))
		{
			return false;
		}

		if (AssetReferenceFilter.IsValid())
		{
			if (!AssetReferenceFilter->PassesFilter(FAssetData(InStruct)))
			{
				return false;
			}
		}

		// Query the native struct to see if it has the correct parent type (if any)
		const UScriptStruct* Struct = BaseStruct.Get();
		return !Struct || InStruct->IsChildOf(Struct);
	}

	virtual bool IsUnloadedStructAllowed(const FStructViewerInitializationOptions& InInitOptions, const FSoftObjectPath& InStructPath, TSharedRef<class FStructViewerFilterFuncs> InFilterFuncs) override
	{
		return false;
	}
};
