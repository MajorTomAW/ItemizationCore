// Copyright Epic Games, Inc. All Rights Reserved.


#include "Toolkits/ItemizationEditorApplication.h"

#include "EditorModes.h"
#include "IDocumentation.h"
#include "ItemizationCoreEditor.h"
#include "ItemDefinition.h"
#include "Commands/ItemizationEditorCommands.h"
#include "Components/ItemComponentData_Equipment.h"
#include "Customization/ItemizationDetailsCustomization.h"
#include "Styles/ItemizationEditorStyle.h"
#include "Toolkits/ItemizationEditorAppMode.h"
#include "Widgets/SItemizationEditorViewport.h"

using namespace UE::ItemizationCore::Editor;

#define LOCTEXT_NAMESPACE "ItemizationEditorApplication"

FItemizationEditorApplication::FItemizationEditorApplication()
{
}

void FItemizationEditorApplication::InitItemizationEditorApp(
	const EToolkitMode::Type Mode, const TSharedPtr<IToolkitHost>& InitToolkitHost, UItemDefinition* InItem)
{
	if (InItem != nullptr)
	{
		ItemDefinition = InItem;
	}

	TSharedPtr<FItemizationEditorApplication> ThisPtr(SharedThis(this));
	TArray<UObject*> ObjectsToEdit;
	ObjectsToEdit.Add(ItemDefinition);

	if (!ToolbarBuilder.IsValid())
	{
		ToolbarBuilder = MakeShareable(new FItemizationEditorToolbar(ThisPtr));
	}
	
	const TArray<UObject*>* EditedObjects = GetObjectsCurrentlyBeingEdited();
	if (EditedObjects == nullptr || EditedObjects->Num() == 0)
	{
		CreateEditorModeManager();
		CreateInternalWidgets();
		
		constexpr bool bCreateDefaultStandaloneMenu = true;
		constexpr bool bCreateDefaultToolbar = true;
		InitAssetEditor(Mode, InitToolkitHost, IDs::ItemizationEditorApp(), FTabManager::FLayout::NullLayout, bCreateDefaultStandaloneMenu, bCreateDefaultToolbar, ObjectsToEdit);

		TArray<IItemizationCoreEditorModule::FOnGetApplicationMode> OnGetModes;
		IItemizationCoreEditorModule& ItemizationModule = IItemizationCoreEditorModule::Get();
		ItemizationModule.FindApplicationModesForAsset(ItemDefinition->GetClass(), OnGetModes);
		UE_LOG(LogEditorModes, Display, TEXT("Found %d mode delegates for asset %s "), OnGetModes.Num(), *GetNameSafe(ItemDefinition));

		for (auto& OnGetMode : OnGetModes)
		{
			if (!OnGetMode.IsBound())
			{
				continue;
			}
			
			TSharedPtr<FApplicationMode> NewMode;
			const IItemizationCoreEditorModule::FItemizationAppModeArgs Args = OnGetMode.Execute(NewMode, ThisPtr);
			UE_LOG(LogTemp, Display, TEXT("     -> Mode Id: %s"), *Args.ModeId.ToString());
			
			if (NewMode.IsValid())
			{
				AddApplicationMode(Args.ModeId, NewMode.ToSharedRef());
				RegisteredApplicationModes.Add(Args.ModeId, NewMode);
			}
		}
	}
	else
	{
		for (UObject* ObjectToEdit : ObjectsToEdit)
		{
			if (!EditedObjects->Contains(ObjectToEdit))
			{
				AddEditingObject(ObjectToEdit);
			}
		}
	}

	if (ItemDefinition != nullptr)
	{
		SetCurrentMode(IDs::AppMode_Default());
	}

	RegenerateMenusAndToolbars();
}

bool FItemizationEditorApplication::CanActivateMode(FName ModeId) const
{
	const TSharedPtr<FApplicationMode> Mode = *GetRegisteredApplicationModes().Find(ModeId);
	if (Mode.IsValid())
	{
		const FItemizationEditorAppMode* ItemMode = static_cast<FItemizationEditorAppMode*>(Mode.Get());
		return ItemMode->CanActivateMode();
	}

	return false;
}

bool FItemizationEditorApplication::CanShowMode(FName ModeId) const
{
	const TSharedPtr<FApplicationMode> Mode = *GetRegisteredApplicationModes().Find(ModeId);
	if (Mode.IsValid())
	{
		const FItemizationEditorAppMode* ItemMode = static_cast<FItemizationEditorAppMode*>(Mode.Get());
		return ItemMode->CanShowMode();
	}

	return false;
}

FName FItemizationEditorApplication::GetToolkitFName() const
{
	return IDs::ItemizationEditorApp();
}

FText FItemizationEditorApplication::GetBaseToolkitName() const
{
	return FText::Format(LOCTEXT("BaseToolkitName", "{0}"), GetToolkitName());
}

FString FItemizationEditorApplication::GetWorldCentricTabPrefix() const
{
	return LOCTEXT("WorldCentricTabPrefix", "Itemization").ToString();
}

FString FItemizationEditorApplication::GetDocumentationLink() const
{
	if (ItemDefinition != nullptr)
	{
		return ItemDefinition->DocumentationURL;
	}

	return FString();
}

void FItemizationEditorApplication::BrowseDocumentation_Execute() const
{
	if (GetDocumentationLink().IsEmpty())
	{
		return;
	}

	IDocumentation::Get()->Open(GetDocumentationLink(), FDocumentationSourceInfo(TEXT("help_menu_asset")));
}

FText FItemizationEditorApplication::GetToolkitName() const
{
	if (ItemDefinition && !ItemDefinition->ItemName.IsEmpty())
	{
		return ItemDefinition->ItemName;
	}
	
	return FWorkflowCentricApplication::GetToolkitName();
}

FText FItemizationEditorApplication::GetToolkitToolTipText() const
{
	FText Tooltip = FWorkflowCentricApplication::GetToolkitToolTipText();
	
	if (ItemDefinition)
	{
		if (!ItemDefinition->ItemShortDescription.IsEmpty())
		{
			Tooltip = FText::FromString(FString::Printf(TEXT("%s\n%s"), *Tooltip.ToString(), *ItemDefinition->ItemShortDescription.ToString()));
			return Tooltip;
		}

		if (!ItemDefinition->ItemDescription.IsEmpty())
		{
			Tooltip = FText::FromString(FString::Printf(TEXT("%s\n\n%s"), *Tooltip.ToString(), *ItemDefinition->ItemDescription.ToString()));
			return Tooltip;
		}
	}
	
	return Tooltip;
}

FLinearColor FItemizationEditorApplication::GetWorldCentricTabColorScale() const
{
	return FItemizationEditorStyle::Get()->GetColor("Colors.TabColorScale");
}

void FItemizationEditorApplication::PostRegenerateMenusAndToolbars()
{
	/*TArray<UClass*> ClassList;
	for (UObject* Obj : GetEditingObjects())
	{
		check(Obj);
		ClassList.Add(Obj->GetClass());
	}

	UClass* CommonDenominatorClass = UClass::FindCommonBase(ClassList);

	TSharedRef<SHorizontalBox> MenuOverlayBox = SNew(SHorizontalBox)
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.VAlign(VAlign_Center)
		[
			SNew(STextBlock)
			.ColorAndOpacity(FSlateColor::UseSubduedForeground())
			.ShadowOffset(FVector2D::UnitVector)
			.Text(LOCTEXT("OverlayBoxLabel", "Asset Type: "))
		]
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.VAlign(VAlign_Center)
		.Padding(0.f, 0.f, 8.f, 0.f)
		[
			FEditorClassUtils::GetSourceLink(CommonDenominatorClass)	
		];

	SetMenuOverlay(MenuOverlayBox);*/
}

void FItemizationEditorApplication::PostInitAssetEditor()
{
	ToolkitCommands->MapAction
	(
		FItemizationEditorCommands::Get().AppMode_Default,
		FExecuteAction::CreateRaw(this, &FItemizationEditorApplication::SetCurrentMode, IDs::AppMode_Default()),
		FCanExecuteAction::CreateRaw(this, &FItemizationEditorApplication::CanActivateMode, IDs::AppMode_Default())
	);

	ToolkitCommands->MapAction
	(
		FItemizationEditorCommands::Get().AppMode_Components,
		FExecuteAction::CreateRaw(this, &FItemizationEditorApplication::SetCurrentMode, IDs::AppMode_Components()),
		FCanExecuteAction::CreateRaw(this, &FItemizationEditorApplication::CanActivateMode, IDs::AppMode_Components())
	);

	ToolkitCommands->MapAction
	(
		FItemizationEditorCommands::Get().AppMode_Equipment,
		FExecuteAction::CreateRaw(this, &FItemizationEditorApplication::SetCurrentMode, IDs::AppMode_Equipment()),
		FCanExecuteAction::CreateRaw(this, &FItemizationEditorApplication::CanActivateMode, IDs::AppMode_Equipment())
	);
}

void FItemizationEditorApplication::SaveAsset_Execute()
{
	FWorkflowCentricApplication::SaveAsset_Execute();

	OnAssetSavedDelegate.Execute();
}

void FItemizationEditorApplication::AddReferencedObjects(FReferenceCollector& Collector)
{
	if (ItemDefinition)
	{
		Collector.AddReferencedObject(ItemDefinition);
	}
}

void FItemizationEditorApplication::NotifyPreChange(FProperty* PropertyAboutToChange)
{
	FNotifyHook::NotifyPreChange(PropertyAboutToChange);
}

void FItemizationEditorApplication::NotifyPostChange(const FPropertyChangedEvent& PropertyChangedEvent, FProperty* PropertyThatChanged)
{
	if (PropertyThatChanged == nullptr || PropertyChangedEvent.Property == nullptr)
	{
		return;
	}

	const TFunction<bool(const FName& InName)> IsProperty = [&](const FName& InName)->bool
	{
		return PropertyThatChanged->GetFName() == InName ||
			PropertyChangedEvent.Property->GetFName() == InName ||
			PropertyThatChanged->GetOwnerProperty()->GetFName() == InName ||
			PropertyChangedEvent.MemberProperty->GetOwnerProperty()->GetFName() == InName;
	};

	const bool bHasPropsChanged =
		IsProperty(GET_MEMBER_NAME_CHECKED(UItemDefinition, DefaultLocation)) ||
		IsProperty(GET_MEMBER_NAME_CHECKED(UItemDefinition, DefaultRotation));

	const bool bHasActorListChanged =
		IsProperty(GET_MEMBER_NAME_CHECKED(UItemDefinition, ActorDataList)) ||
		IsProperty(GET_MEMBER_NAME_CHECKED(UItemDefinition, SourceActorBlueprint));

	if (Viewport.IsValid() && (bHasPropsChanged || bHasActorListChanged))
	{
		Viewport->UpdateViewport(ItemDefinition, bHasActorListChanged);
	}
}

void FItemizationEditorApplication::PostUndo(bool bSuccess)
{
	FSelfRegisteringEditorUndoClient::PostUndo(bSuccess);
}

void FItemizationEditorApplication::PostRedo(bool bSuccess)
{
	FSelfRegisteringEditorUndoClient::PostRedo(bSuccess);
}

void FItemizationEditorApplication::CreateInternalWidgets()
{
	FPropertyEditorModule& PropEdMod = FModuleManager::GetModuleChecked<FPropertyEditorModule>("PropertyEditor");
	TSharedPtr<FItemizationEditorApplication> ThisPtr(SharedThis(this));

	FDetailsViewArgs ViewArgs;
	{
		ViewArgs.NameAreaSettings = FDetailsViewArgs::HideNameArea;
		ViewArgs.NotifyHook = this;
		ViewArgs.DefaultsOnlyVisibility = EEditDefaultsOnlyNodeVisibility::Show;
		ViewArgs.bAllowFavoriteSystem = false;
		ViewArgs.bShowPropertyMatrixButton = false;
	}

	if (!Viewport.IsValid())
	{
		SEditorViewport::FArguments ViewportArgs = SEditorViewport::FArguments()
		.IsEnabled(FSlateApplication::Get().GetNormalExecutionAttribute())
		.Cursor(EMouseCursor::Crosshairs)
		.AddMetaData<FTagMetaData>(FTagMetaData(TEXT("ItemizationEditorViewport")));

		Viewport = SNew(SItemizationEditorViewport).ViewportArgs(ViewportArgs);
	}

	if (!DetailsView.IsValid())
	{
		DetailsView = PropEdMod.CreateDetailView(ViewArgs);
		DetailsView->SetObject(nullptr);
		DetailsView->ClearSearch();

		DetailsView->RegisterInstancedCustomPropertyLayout(UItemDefinition::StaticClass(), FOnGetDetailCustomizationInstance::CreateRaw(this, &FItemizationEditorApplication::GetDetailsCustomization));
	}

	if (!DisplayInfoView.IsValid())
	{
		DisplayInfoView = PropEdMod.CreateDetailView(ViewArgs);
		DisplayInfoView->SetObject(nullptr);
		DisplayInfoView->ClearSearch();

		DisplayInfoView->RegisterInstancedCustomPropertyLayout(UItemDefinition::StaticClass(), FOnGetDetailCustomizationInstance::CreateRaw(this, &FItemizationEditorApplication::GetDisplayInfoCustomization));
	}

	if (!PlacementView.IsValid())
	{
		PlacementView = PropEdMod.CreateDetailView(ViewArgs);
		PlacementView->SetObject(nullptr);
		PlacementView->ClearSearch();

		PlacementView->RegisterInstancedCustomPropertyLayout(UItemDefinition::StaticClass(), FOnGetDetailCustomizationInstance::CreateRaw(this, &FItemizationEditorApplication::GetPlacementCustomization));
	}

	if (!DevelopmentView.IsValid())
	{
		DevelopmentView = PropEdMod.CreateDetailView(ViewArgs);
		DevelopmentView->SetObject(nullptr);
		DevelopmentView->ClearSearch();
		DevelopmentView->SetIsPropertyVisibleDelegate(FIsPropertyVisible::CreateLambda([&](const FPropertyAndParent& PropAndParent)->bool
		{
			if (ItemDefinition == nullptr)
			{
				return false;
			}
			
			if (PropAndParent.Property.GetName() == GET_MEMBER_NAME_CHECKED(UItemDefinition, DefaultEquipmentInstanceClass))
			{
				return ItemDefinition->GetItemComponent<FItemComponentData_Equipment>() != nullptr;
			}

			return true;
		}));
		DevelopmentView->SetIsPropertyReadOnlyDelegate(FIsPropertyReadOnly::CreateLambda([&](const FPropertyAndParent& PropAndParent)->bool
		{
			if (ItemDefinition == nullptr)
			{
				return false;
			}

			if (PropAndParent.Property.GetName() == GET_MEMBER_NAME_CHECKED(UItemDefinition, DefaultEquipmentInstanceClass))
			{
				const FItemComponentData_Equipment* Data_Equipment = ItemDefinition->GetItemComponent<FItemComponentData_Equipment>();
				if (Data_Equipment == nullptr)
				{
					return true;
				}

				return !Data_Equipment->bCanBeEquipped;
			}

			return false;
		}));

		DevelopmentView->RegisterInstancedCustomPropertyLayout(UItemDefinition::StaticClass(), FOnGetDetailCustomizationInstance::CreateRaw(this, &FItemizationEditorApplication::GetDevelopmentCustomization));
	}

	if (!ComponentsView.IsValid())
	{
		ComponentsView = PropEdMod.CreateDetailView(ViewArgs);
		ComponentsView->SetObject(nullptr);
		ComponentsView->ClearSearch();

		ComponentsView->RegisterInstancedCustomPropertyLayout(UItemDefinition::StaticClass(), FOnGetDetailCustomizationInstance::CreateRaw(this, &FItemizationEditorApplication::GetComponentsCustomization));
	}

	if (!EquipmentView.IsValid())
	{
		EquipmentView = PropEdMod.CreateDetailView(ViewArgs);
		EquipmentView->SetObject(nullptr);
		EquipmentView->ClearSearch();

		EquipmentView->RegisterInstancedCustomPropertyLayout(UItemDefinition::StaticClass(), FOnGetDetailCustomizationInstance::CreateRaw(this, &FItemizationEditorApplication::GetEquipmentCustomization));
	}
}

TSharedRef<IDetailCustomization> FItemizationEditorApplication::GetDetailsCustomization()
{
	return FItemizationDetailsCustomization::MakeInstance(FItemizationDetailsViewArgs::DetailsView(), SharedThis(this));
}

TSharedRef<IDetailCustomization> FItemizationEditorApplication::GetDisplayInfoCustomization()
{
	return FItemizationDetailsCustomization::MakeInstance(FItemizationDetailsViewArgs::DisplayInfo(), SharedThis(this));
}

TSharedRef<IDetailCustomization> FItemizationEditorApplication::GetPlacementCustomization()
{
	return FItemizationDetailsCustomization::MakeInstance(FItemizationDetailsViewArgs::Placement(), SharedThis(this));
}

TSharedRef<IDetailCustomization> FItemizationEditorApplication::GetDevelopmentCustomization()
{
	return FItemizationDetailsCustomization::MakeInstance(FItemizationDetailsViewArgs::Development(), SharedThis(this));
}

TSharedRef<IDetailCustomization> FItemizationEditorApplication::GetComponentsCustomization()
{
	return FItemizationDetailsCustomization::MakeInstance(FItemizationDetailsViewArgs::Components(), SharedThis(this));
}

TSharedRef<IDetailCustomization> FItemizationEditorApplication::GetEquipmentCustomization()
{
	return FItemizationDetailsCustomization::MakeInstance(FItemizationDetailsViewArgs::Equipment(), SharedThis(this));
}

TSharedRef<SWidget> FItemizationEditorApplication::SpawnTab_Details() const
{
	return SNew(SHorizontalBox)
		+ SHorizontalBox::Slot()
		.FillWidth(1.f)
		.HAlign(HAlign_Fill)
		[
			DetailsView.ToSharedRef()	
		];
}

TSharedRef<SWidget> FItemizationEditorApplication::SpawnTab_DisplayInfo() const
{
	return SNew(SHorizontalBox)
		+ SHorizontalBox::Slot()
		.FillWidth(1.f)
		.HAlign(HAlign_Fill)
		[
			DisplayInfoView.ToSharedRef()
		];
}

TSharedRef<SWidget> FItemizationEditorApplication::SpawnTab_Placement() const
{
	return SNew(SHorizontalBox)
		+ SHorizontalBox::Slot()
		.FillWidth(1.f)
		.HAlign(HAlign_Fill)
		[
			PlacementView.ToSharedRef()
		];
}

TSharedRef<SWidget> FItemizationEditorApplication::SpawnTab_Development() const
{
	return SNew(SHorizontalBox)
		+ SHorizontalBox::Slot()
		.FillWidth(1.f)
		.HAlign(HAlign_Fill)
		[
			DevelopmentView.ToSharedRef()
		];
}

TSharedRef<SWidget> FItemizationEditorApplication::SpawnTab_Components() const
{
	return SNew(SVerticalBox)
		+ SVerticalBox::Slot()
		.FillHeight(1.f)
		.HAlign(HAlign_Fill)
		[
			ComponentsView.ToSharedRef()
		];
}

TSharedRef<SWidget> FItemizationEditorApplication::SpawnTab_Equipment() const
{
	return SNew(SHorizontalBox)
		+ SHorizontalBox::Slot()
		.FillWidth(1.f)
		.HAlign(HAlign_Fill)
		[
			EquipmentView.ToSharedRef()
		];
}

TSharedRef<SWidget> FItemizationEditorApplication::SpawnTab_Viewport() const
{
	TSharedRef<SWidget> SpawnedTab = SNew(SVerticalBox)
		+ SVerticalBox::Slot()
		.FillHeight(1.f)
		.HAlign(HAlign_Fill)
		[
			Viewport.ToSharedRef()	
		];

	Viewport->UpdateViewport(ItemDefinition);
	return SpawnedTab;
}

#undef LOCTEXT_NAMESPACE
