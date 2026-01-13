// Author: Tom Werner (MajorT), 2025 November


#include "SItemDataListView.h"

#include "ItemDefinitionAppUserSettings.h"
#include "ItemizationEditorHelpers.h"
#include "PropertyCustomizationHelpers.h"
#include "SItemDataListRow.h"
#include "SlateOptMacros.h"
#include "SPositiveActionButton.h"
#include "Commands/ItemDefinitionAppCommands.h"
#include "Items/ItemDefinitionBase.h"
#include "Toolkits/ItemDefinitionViewModel.h"

#define LOCTEXT_NAMESPACE "SItemDataListView"

SItemDataListView::SItemDataListView()
	: bItemsDirty(false)
	, bUpdatingSelection(false)
{
}

SItemDataListView::~SItemDataListView()
{
	if (UObjectInitialized())
	{
		if (ItemViewModel)
		{
			ItemViewModel->GetOnAssetChanged().RemoveAll(this);
			ItemViewModel->GetOnItemDataAdded().RemoveAll(this);
			ItemViewModel->GetOnItemDataRemoved().RemoveAll(this);
			ItemViewModel->GetOnItemDataChanged().RemoveAll(this);
			ItemViewModel->GetOnSelectionChanged().RemoveAll(this);
		}
	}
}

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION
void SItemDataListView::Construct(
	const FArguments& InArgs,
	TSharedRef<FItemDefinitionViewModel> InItemViewModel,
	const TSharedRef<FUICommandList>& InCommandList)
{
	ItemViewModel = InItemViewModel;
	ItemViewModel->GetOnAssetChanged().AddSP(this, &ThisClass::HandleModelAssetChanged);
	ItemViewModel->GetOnItemDataAdded().AddSP(this, &ThisClass::HandleModelItemDataAdded);
	ItemViewModel->GetOnItemDataChanged().AddSP(this, &ThisClass::HandleModelItemDataChanged);
	ItemViewModel->GetOnItemDataRemoved().AddSP(this, &ThisClass::HandleModelItemDataRemoved);
	ItemViewModel->GetOnSelectionChanged().AddSP(this, &ThisClass::HandleModelSelectionChanged);

	bUpdatingSelection = false;

	TSharedRef<SScrollBar> HorizontalScrollBar = SNew(SScrollBar)
		.Orientation(Orient_Horizontal)
		.Thickness(FVector2D(12.f));

	TSharedRef<SScrollBar> VerticalScrollBar = SNew(SScrollBar)
		.Orientation(Orient_Vertical)
		.Thickness(FVector2D(12.f));

	ItemViewModel->GetDataList(ItemDataList);
	UE_LOG(LogTemp, Warning, TEXT("DataList: %d"), ItemDataList.Num())

	ListView = SNew(SListView<TSharedPtr<FItemDataUIInfo>>)
		.OnGenerateRow(this, &ThisClass::HandleGenerateRow)
		.ListItemsSource(&ItemDataList)
		.OnSelectionChanged(this, &ThisClass::HandleListSelectionChanged)
		.OnContextMenuOpening(this, &ThisClass::HandleListContextMenuOpening)
		.AllowOverscroll(EAllowOverscroll::Yes)
		.SelectionMode(ESelectionMode::Type::Single)
		.ExternalScrollbar(VerticalScrollBar);

	ChildSlot
	[
		SNew(SVerticalBox)

		// Header
		+ SVerticalBox::Slot()
		.VAlign(VAlign_Center)
		.AutoHeight()
		[
			SNew(SBorder)
			.BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
			.Padding(8.f, 4.f, 8.f, 2.f)
			[
				SNew(SHorizontalBox)

				// New Item Data
				+ SHorizontalBox::Slot()
				.VAlign(VAlign_Center)
				//.Padding(4.f, 4.f, 4.f, 2.f)
				.AutoWidth()
				[
					SAssignNew(ComboButton, SComboButton)
					.ButtonStyle(FAppStyle::Get(), "SimpleButton")
					.ToolTipText(LOCTEXT("AddItemDataTooltip", "Add New Item Data"))
					.ContentPadding(0.f)
					.HasDownArrow(false)
					.OnGetMenuContent(this, &ThisClass::HandleGenerateStructPicker)
					.ButtonContent()
					[
						SNew(SHorizontalBox)

						+ SHorizontalBox::Slot()
						.HAlign(HAlign_Center)
						.VAlign(VAlign_Center)
						.AutoWidth()
						.Padding(2.f)
						[
							SNew(SImage)
							.Image(FAppStyle::Get().GetBrush("Icons.PlusCircle"))
							.ColorAndOpacity(FStyleColors::AccentGreen)
						]

						+ SHorizontalBox::Slot()
						.VAlign(VAlign_Center)
						.FillWidth(1.f)
						.Padding(2.f)
						[
							SNew(STextBlock)
							.Text(LOCTEXT("AddItemData", "Add Item Data"))
						]
					]
				]

				+ SHorizontalBox::Slot()
				.FillWidth(1.f)
				[
					SNew(SSpacer)
				]

				+ SHorizontalBox::Slot()
				.AutoWidth()
				.HAlign(HAlign_Right)
				[
					SNew(SComboButton)
					.HasDownArrow(false)
					.ContentPadding(0.f)
					.ButtonStyle(FAppStyle::Get(), "SimpleButton")
					.MenuContent()
					[
						HandleGenerateSettingsMenu()
					]
					.ButtonContent()
					[
						SNew(SHorizontalBox)

						+ SHorizontalBox::Slot()
						.HAlign(HAlign_Center)
						.VAlign(VAlign_Center)
						.AutoWidth()
						.Padding(2.f)
						[
							SNew(SImage)
							.Image(FAppStyle::Get().GetBrush("DetailsView.ViewOptions"))
						]

						+ SHorizontalBox::Slot()
						.VAlign(VAlign_Center)
						.FillWidth(1.f)
						.Padding(2.f)
						[
							SNew(STextBlock)
							.Text(LOCTEXT("ViewOptions", "View Options"))
						]
					]
				]
			]
		]

		// Body
		+ SVerticalBox::Slot()
		.Padding(2.f, 6.f, 2.f, 2.f)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.FillWidth(1.f)
			.Padding(0.f)
			[
				SAssignNew(ViewBox, SScrollBox)
				.Orientation(Orient_Horizontal)
				.ExternalScrollbar(HorizontalScrollBar)

				+ SScrollBox::Slot()
				.FillSize(1.f)
				[
					ListView.ToSharedRef()
				]
			]

			+ SHorizontalBox::Slot()
			.AutoWidth()
			[
				VerticalScrollBar
			]
		]

		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			HorizontalScrollBar
		]
	];

	CommandList = InCommandList;
	BindCommands();
}
END_SLATE_FUNCTION_BUILD_OPTIMIZATION

TSharedPtr<FItemDefinitionViewModel> SItemDataListView::GetViewModel() const
{
	return ItemViewModel;
}

void SItemDataListView::SetSelection(const TArray<TSharedPtr<FItemDataUIInfo>>& SelectedData) const
{
}

void SItemDataListView::BindCommands()
{
	const FItemDefinitionAppCommands& Cmd = FItemDefinitionAppCommands::Get();

	CommandList->MapAction(
		Cmd.DeleteItemData,
		FExecuteAction::CreateSP(this, &ThisClass::HandleDeleteItemData),
		FCanExecuteAction::CreateSP(this, &SItemDataListView::HasSelection));
}

FReply SItemDataListView::OnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent)
{
	if (CommandList->ProcessCommandBindings(InKeyEvent))
	{
		return FReply::Handled();
	}

	return SCompoundWidget::OnKeyDown(MyGeometry, InKeyEvent);
}

void SItemDataListView::Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime)
{
	if (bItemsDirty)
	{
		UpdateList();
	}
}

void SItemDataListView::UpdateList()
{
	if (!ItemViewModel)
	{
		return;
	}

	auto Selection = ItemViewModel->GetSelection();

	// Regenerate items
	ItemViewModel->GetDataList(ItemDataList);
	ListView->SetItemsSource(&ItemDataList);

	// Restore selection
	ListView->ClearSelection();
	ListView->SetItemSelection(Selection, true);

	ListView->RequestListRefresh();

	bItemsDirty = false;
}

void SItemDataListView::HandleModelAssetChanged()
{
	bItemsDirty = true;
	ListView->RebuildList();
}

void SItemDataListView::HandleModelItemDataChanged(
	const TSharedPtr<FItemDataUIInfo>& AffectedItemData,
	const FPropertyChangedEvent& ChangedEvent)
{
	auto CachedSelection = ItemViewModel->GetSelection();
	TArray<TSharedPtr<FItemDataUIInfo>> EmptySelection;

	bUpdatingSelection = true;
	ItemViewModel->SetSelection(EmptySelection);
	bUpdatingSelection = false;

	bUpdatingSelection = true;
	ItemViewModel->SetSelection(CachedSelection);
	bUpdatingSelection = false;
}

void SItemDataListView::HandleModelItemDataAdded(TSharedPtr<FItemDataUIInfo>& AddedItemData)
{
	bItemsDirty = true;

	if (ListView.IsValid())
	{
		ListView->SetSelection(AddedItemData);
	}
}

void SItemDataListView::HandleModelItemDataRemoved()
{
	bItemsDirty = true;
}

void SItemDataListView::HandleModelSelectionChanged(
	const TArray<TSharedPtr<FItemDataUIInfo>>& Selection)
{
	if (bUpdatingSelection)
	{
		return;
	}

	ListView->ClearSelection();

	if (!Selection.IsEmpty())
	{
		ListView->SetItemSelection(Selection, true);

		if (Selection.Num() == 1)
		{
			ListView->RequestScrollIntoView(Selection[0]);
		}
	}
}

TSharedRef<ITableRow> SItemDataListView::HandleGenerateRow(
	TSharedPtr<FItemDataUIInfo> InUIInfo,
	const TSharedRef<STableViewBase>& InOwnerTableView)
{
	return SNew(SItemDataListRow, InOwnerTableView, InUIInfo, ViewBox, ItemViewModel.ToSharedRef());
}

void SItemDataListView::HandleListSelectionChanged(
	TSharedPtr<FItemDataUIInfo> InSelectedItem,
	ESelectInfo::Type SelectionType)
{
	if (!ItemViewModel)
	{
		return;
	}

	// Do not report code based selection changes.
	if (SelectionType == ESelectInfo::Direct)
	{
		return;
	}

	TArray<TSharedPtr<FItemDataUIInfo>> SelectedItems = ListView->GetSelectedItems();
	bUpdatingSelection = true;
	ItemViewModel->SetSelection(SelectedItems);
	bUpdatingSelection = false;
}

TSharedRef<SWidget> SItemDataListView::HandleGenerateStructPicker()
{
	static const FName NAME_ExcludeBaseStruct = "ExcludeBaseStruct";
	static const FName NAME_HideViewOptions = "HideViewOptions";
	static const FName NAME_ShowTreeView = "ShowTreeView";

	UItemDefinitionBase* ItemDefinition = ItemViewModel->GetItemDefinition();

	/*const bool bExcludeBaseStruct = InstanceProperty->HasMetaData(NAME_ExcludeBaseStruct);
	const bool bAllowNone = !(InstanceProperty->GetMetaDataProperty()->PropertyFlags & CPF_NoClear);
	const bool bHideViewOptions = InstanceProperty->HasMetaData(NAME_HideViewOptions);
	const bool bShowTreeView = InstanceProperty->HasMetaData(NAME_ShowTreeView);*/

	constexpr bool bExcludeBaseStruct = true;
	constexpr bool bAllowNone = false;
	constexpr bool bHideViewOptions = true;
	constexpr bool bShowTreeView = false;

	using namespace UE::ItemizationEditor;
	TSharedRef<FItemComponentDataStructFilter> Filter = MakeShared<FItemComponentDataStructFilter>();
	Filter->BaseStruct = FItemComponentData::StaticStruct();
	Filter->bAllowBaseStruct = !bExcludeBaseStruct;
	Filter->bAllowUserDefinedStructs = false;

	if (IsValid(ItemDefinition))
	{
		TArray<TSoftObjectPtr<const UScriptStruct>> DisallowedDataTypes = ItemDefinition->GetDisallowedDataTypes();
		if (!DisallowedDataTypes.IsEmpty())
		{
			Filter->DisallowedStructs = DisallowedDataTypes;
		}
	}

	// Construct our filter
	if (GEditor && ItemDefinition)
	{
		FAssetReferenceFilterContext AssetReferenceFilterContext;

		UPackage* OuterPackage = ItemDefinition->GetPackage();
		AssetReferenceFilterContext.AddReferencingAsset(FAssetData(OuterPackage));

		Filter->AssetReferenceFilter = GEditor->MakeAssetReferenceFilter(AssetReferenceFilterContext);

		/*auto SoftPointerTransform = [](const UScriptStruct* InStruct) -> TSoftObjectPtr<const UScriptStruct>
		{
			return InStruct;
		};

		Algo::Transform(PropertyCustomizationHelpers::GetStructsFromMetadataString(InstanceProperty->GetMetaData("AllowedClasses")), Filter->AllowedStructs, SoftPointerTransform);
		Algo::Transform(PropertyCustomizationHelpers::GetStructsFromMetadataString(InstanceProperty->GetMetaData("DisallowedClasses")), Filter->DisallowedStructs, SoftPointerTransform);

		TArray<UObject*> OwningObjects;
		InstanceProperty->GetOuterObjects(OwningObjects);
		for (UObject* OwningObject : OwningObjects)
		{
			if (OwningObject != nullptr)
			{
				const FString GetAllowedClassesFunctionName = InstanceProperty->GetMetaData("GetAllowedClasses");
				if (!GetAllowedClassesFunctionName.IsEmpty())
				{
					const UFunction* GetAllowedClassesFunction = OwningObject ? OwningObject->FindFunction(*GetAllowedClassesFunctionName) : nullptr;
					if (GetAllowedClassesFunction != nullptr)
					{
						DECLARE_DELEGATE_RetVal(TArray<TSoftObjectPtr<UScriptStruct>>, FGetAllowedClasses);
						Filter->AllowedStructs.Append(FGetAllowedClasses::CreateUFunction(OwningObject, GetAllowedClassesFunction->GetFName()).Execute());
					}
				}

				const FString GetDisallowedClassesFunctionName = InstanceProperty->GetMetaData("GetDisallowedClasses");
				if (!GetDisallowedClassesFunctionName.IsEmpty())
				{
					const UFunction* GetDisallowedClassesFunction = OwningObject ? OwningObject->FindFunction(*GetDisallowedClassesFunctionName) : nullptr;
					if (GetDisallowedClassesFunction != nullptr)
					{
						DECLARE_DELEGATE_RetVal(TArray<TSoftObjectPtr<UScriptStruct>>, FGetDisallowedClasses);
						Filter->DisallowedStructs.Append(FGetDisallowedClasses::CreateUFunction(OwningObject, GetDisallowedClassesFunction->GetFName()).Execute());
					}
				}
			}
		}*/
	}


	FStructViewerInitializationOptions Options;
	Options.bShowNoneOption = bAllowNone;
	Options.StructFilter = Filter;
	Options.NameTypeToDisplay = EStructViewerNameTypeToDisplay::DisplayName;
	Options.DisplayMode = bShowTreeView ? EStructViewerDisplayMode::TreeView : EStructViewerDisplayMode::ListView;
	Options.bAllowViewOptions = !bHideViewOptions;
	/*Options.SelectedStruct = SelectedStruct;
	Options.PropertyHandle = StructProperty;*/

	const FOnStructPicked OnPicked(FOnStructPicked::CreateSP(this, &ThisClass::HandleStructPicked));

	return SNew(SBox)
		.WidthOverride(280.f)
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.AutoHeight()
			.MaxHeight(500.f)
			[
				FModuleManager::LoadModuleChecked<FStructViewerModule>("StructViewer").CreateStructViewer(Options, OnPicked)
			]
		];
}

void SItemDataListView::HandleStructPicked(const UScriptStruct* InStruct)
{
	ItemViewModel->AddItemData(InStruct);
	ComboButton->SetIsOpen(false);
}

TSharedRef<SWidget> SItemDataListView::HandleGenerateSettingsMenu()
{
	FDetailsViewArgs DetailsViewArgs;
	DetailsViewArgs.bUpdatesFromSelection = false;
	DetailsViewArgs.bLockable = false;
	DetailsViewArgs.bShowPropertyMatrixButton = false;
	DetailsViewArgs.bAllowSearch = false;
	DetailsViewArgs.bShowOptions = false;
	DetailsViewArgs.NameAreaSettings = FDetailsViewArgs::HideNameArea;
	DetailsViewArgs.ViewIdentifier = NAME_None;

	FPropertyEditorModule& PropertyEditorModule = FModuleManager::GetModuleChecked<FPropertyEditorModule>("PropertyEditor");
	TSharedRef<IDetailsView> DetailsView = PropertyEditorModule.CreateDetailView(DetailsViewArgs);
	DetailsView->SetObject(UItemDefinitionAppUserSettings::Get());

	return SNew(SBox)
		.MinDesiredWidth(256.f)
		[
			DetailsView
		];
}

TSharedPtr<SWidget> SItemDataListView::HandleListContextMenuOpening()
{
	if (!ItemViewModel)
	{
		return nullptr;
	}

	FMenuBuilder MenuBuilder(true, CommandList);
	MenuBuilder.AddMenuEntry(FItemDefinitionAppCommands::Get().DeleteItemData);

	return MenuBuilder.MakeWidget();
}

bool SItemDataListView::HasSelection() const
{
	return ItemViewModel && ItemViewModel->HasSelection();
}

void SItemDataListView::HandleDeleteItemData()
{
	if (ItemViewModel)
	{
		ItemViewModel->RemoveSelectedItemData();
	}
}

#undef LOCTEXT_NAMESPACE
