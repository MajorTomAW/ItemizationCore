// Author: Tom Werner (MajorT), 2025 November

#pragma once
#include "IDetailCustomization.h"

class FItemDefinitionApp;

namespace UE::ItemizationEditor
{
	struct FAssetDetailsViewArgs
	{
		friend class FAssetDetailsViewCustomization;

		static FAssetDetailsViewArgs AssetDetailsView()
		{
			FAssetDetailsViewArgs Args;
			Args.CategoryBlacklist.Append({"General", "Data", "Asset"});
			return Args;
		}

		static FAssetDetailsViewArgs SettingsDetailsView()
		{
			FAssetDetailsViewArgs Args;
			Args.CategoryWhitelist.Append({"General", "Asset"});
			return Args;
		}

	private:
		TArray<FName> CategoryBlacklist;
		TArray<FName> CategoryWhitelist;
	};

	class FAssetDetailsViewCustomization : public IDetailCustomization
	{
	public:
		FAssetDetailsViewCustomization();
		static TSharedRef<IDetailCustomization> MakeInstance(const FAssetDetailsViewArgs& InArgs, const TSharedPtr<FItemDefinitionApp>& InApp);

	protected:
		//~ Begin IDetailCustomization Interface
		virtual void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) override;
		//~ End IDetailCustomization Interface

	protected:
		FAssetDetailsViewArgs Args;
		TWeakPtr<FItemDefinitionApp> WeakApp;
	};
}
