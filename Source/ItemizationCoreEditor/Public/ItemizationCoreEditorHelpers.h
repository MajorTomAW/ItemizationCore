// Copyright © 2024 MajorT. All Rights Reserved.

#pragma once

#include "ClassViewerFilter.h"
#include "AssetRegistry/AssetRegistryModule.h"

namespace UE::ItemizationCore::Editor
{
	inline bool DoesClassHaveSubtypes(const UClass* Class)
	{
		// Search for native classes
		for (TObjectIterator<UClass> It; It; ++It)
		{
			if (!It->IsNative() || !It->IsChildOf(Class))
			{
				continue;
			}

			if (It->HasAnyClassFlags(CLASS_Abstract | CLASS_HideDropDown | CLASS_Deprecated | CLASS_NewerVersionExists))
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

	/** Defines a set of rules for a given asset type of item definition */
	class ITEMIZATIONCOREEDITOR_API FItemizationEditorAssetRuleSet : public TSharedFromThis<FItemizationEditorAssetRuleSet>
	{
	public:
		friend class FItemizationEditorAssetConfig;
		void AddAllowedItemComponent(FName ComponentName)
		{
			AllowedItemComponents.Add(ComponentName);
		}
		void AddAllowedItemComponents(const TArray<FName>& ComponentNames)
		{
			AllowedItemComponents.Append(ComponentNames);
		}
		
		void AddDisallowedItemComponent(FName ComponentName)
		{
			DisallowedItemComponents.Add(ComponentName);
		}
		void AddDisallowedItemComponents(const TArray<FName>& ComponentNames)
		{
			DisallowedItemComponents.Append(ComponentNames);
		}
		
		void AddAllowedModeId(FName ModeId)
		{
			AllowedModeIds.Add(ModeId);
		}
		void AddAllowedModeIds(const TArray<FName>& ModeIds)
		{
			AllowedModeIds.Append(ModeIds);
		}
		
		void AddDisallowedModeId(FName ModeId)
		{
			DisallowedModeIds.Add(ModeId);
		}
		void AddDisallowedModeIds(const TArray<FName>& ModeIds)
		{
			DisallowedModeIds.Append(ModeIds);
		}

	protected:
		//-------------------------------------------------------------------------
		// Black & White Lists
		//-------------------------------------------------------------------------

		/** The set of allowed item components for the given asset. */
		TSet<FName> AllowedItemComponents;

		/** The set of disallowed item components for the given asset. */
		TSet<FName> DisallowedItemComponents;

		/** The set of allowed editor modes for the given asset. */
		TSet<FName> AllowedModeIds;

		/** The set of disallowed editor modes for the given asset. */
		TSet<FName> DisallowedModeIds;
	};

	class ITEMIZATIONCOREEDITOR_API FItemizationEditorAssetConfig : public TSharedFromThis<FItemizationEditorAssetConfig>
	{
	public:
		/** The asset name that this config is for */
		FName AssetName;

		void SetRuleSet(const TSharedPtr<FItemizationEditorAssetRuleSet>& InRuleSet)
		{
			RuleSet = InRuleSet;
		}

		bool CanShowAppMode(const FName& ModeId) const
		{
			if (!RuleSet->AllowedModeIds.IsEmpty())
			{
				return RuleSet->AllowedModeIds.Contains(ModeId);
			}
			
			return !RuleSet->DisallowedModeIds.Contains(ModeId);
		}

		bool CanShowItemComponent(const FName& ComponentName) const
		{
			if (!RuleSet->AllowedItemComponents.IsEmpty())
			{
				return RuleSet->AllowedItemComponents.Contains(ComponentName);
			}
			
			return !RuleSet->DisallowedItemComponents.Contains(ComponentName);
		}
		
	protected:
		/** The rule set for this asset */
		TSharedPtr<FItemizationEditorAssetRuleSet> RuleSet;
	};
}