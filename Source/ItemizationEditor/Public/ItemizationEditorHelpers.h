// Author: Tom Werner (MajorT), 2025 November

#pragma once
#include "ClassViewerFilter.h"
#include "ClassViewerModule.h"
#include "StructViewerFilter.h"
#include "StructViewerModule.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "StructUtils/UserDefinedStruct.h"

namespace UE::ItemizationEditor
{
	inline bool DoesClassHaveSubtypes(const UClass* Class)
	{
		// Search for native classes first
		for (TObjectIterator<UClass> It; It; ++It)
		{
			if (!It->IsNative() || !It->IsChildOf(Class))
			{
				continue;
			}

			if (!It->HasAnyClassFlags(CLASS_Abstract | CLASS_HideDropDown | CLASS_Deprecated | CLASS_NewerVersionExists))
			{
				continue;
			}

			const UObject* CDO = It->GetDefaultObject();
			if (CDO && CDO->GetClass() != Class)
			{
				return true;
			}
		}

		// Search for blueprints via asset registry
		const FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(FName("AssetRegistry"));
		IAssetRegistry& AssetRegistry = AssetRegistryModule.Get();

		FARFilter Filter;
		Filter.ClassPaths.Add(UBlueprint::StaticClass()->GetClassPathName());
		Filter.bRecursiveClasses = true;
		TArray<FAssetData> BlueprintAssets;

		AssetRegistry.GetAssets(Filter, BlueprintAssets);

		for (FAssetData& Asset : BlueprintAssets)
		{
			FAssetDataTagMapSharedView::FFindTagResult Result = Asset.TagsAndValues.FindTag(TEXT("NativeParentClass"));
			if (!Result.IsSet())
			{
				continue;
			}

			const FString ClassObjectPath = FPackageName::ExportTextPathToObjectPath(Result.GetValue());
			if (const UClass* ParentClass = FindObjectSafe<UClass>(nullptr, *ClassObjectPath, true))
			{
				if (ParentClass->IsChildOf(Class))
				{
					return true;
				}
			}
		}

		return false;
	}


	class FItemizationClassViewerFilter : public IClassViewerFilter
	{
	public:
		FItemizationClassViewerFilter()
			: DisallowedClassFlags(CLASS_Abstract | CLASS_Deprecated | CLASS_NewerVersionExists | CLASS_HideDropDown)
		{
		}

		explicit FItemizationClassViewerFilter(const UClass* AllowedClass)
			: DisallowedClassFlags(CLASS_Abstract | CLASS_Deprecated | CLASS_NewerVersionExists | CLASS_HideDropDown)
		{
			AllowedChildrenOfClasses.Add(AllowedClass);
		}

		TSet<const UClass*> AllowedChildrenOfClasses;
		EClassFlags DisallowedClassFlags;

	public:
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
}
