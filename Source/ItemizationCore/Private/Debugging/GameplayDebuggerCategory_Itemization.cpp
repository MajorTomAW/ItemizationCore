// Author: Tom Werner (dc: majort), 2026


#include "GameplayDebuggerCategory_Itemization.h"

#if WITH_GAMEPLAY_DEBUGGER_MENU

#include "InventoryBase.h"
#include "InventoryLibrary.h"
#include "Engine/Canvas.h"
const FString FGameplayDebuggerCategory_Itemization::NetFilterStrings[(int32)ENetworkStatus::MAX] =
	{
		TEXT("Server Only"),
		TEXT("Local Only"),
		TEXT("Both"),
	};

const FString FGameplayDebuggerCategory_Itemization::NetFilterColors[(int32)ENetworkStatus::MAX] =
	{
		TEXT("cyan"),
		TEXT("red"),
		TEXT("yellow")
	};

const FString LongestDebugObjectName{ TEXT("ABCDEFGHIJKLMNOPQRSTUVWXYZ_ABCDEFGH") };

FGameplayDebuggerCategory_Itemization::FGameplayDebuggerCategory_Itemization()
{
	SetDataPackReplication<FRepData>(&DataPack);

	// Hardcoding these to avoid needing to import InputCore just for EKeys::GetFName();
	static const FName NAME_KeyOne{"One"};
	static const FName NAME_KeyTwo{"Two"};
	static const FName NAME_KeyT{"T"};


	BindKeyPress(NAME_KeyOne, FGameplayDebuggerInputModifier::Shift, this, &ThisClass::ToggleShowItems);
	BindKeyPress(NAME_KeyTwo, FGameplayDebuggerInputModifier::Shift, this, &ThisClass::ToggleShowSlots);
	BindKeyPress(NAME_KeyT, FGameplayDebuggerInputModifier::Shift, this, &ThisClass::ToggleNetworkFiler);
}

TSharedRef<FGameplayDebuggerCategory> FGameplayDebuggerCategory_Itemization::MakeInstance()
{
	return MakeShared<ThisClass>();
}

void FGameplayDebuggerCategory_Itemization::CollectData(
	APlayerController* OwnerPC, AActor* DebugActor)
{
	if (!IsValid(DebugActor))
	{
		return;
	}

	AInventoryBase* Inventory = UInventoryLibrary::FindInventory(DebugActor);
	if (IsValid(Inventory))
	{
		// cache the net package map
		const UNetConnection* NetConnection = OwnerPC->GetNetConnection();
		DataPack.ClientPackageMap = NetConnection ? NetConnection->PackageMap : nullptr;

		// get inventory data
		DataPack.InventoryActor = GetNameSafe(Inventory);
		DataPack.NetOwner = GetNameSafe(Inventory->GetNetOwner());

		// Items
		DataPack.Items = CollectItems(OwnerPC, Inventory);

		// Slots
		DataPack.Slots = CollectSlots(OwnerPC, Inventory);
	}
}

void FGameplayDebuggerCategory_Itemization::DrawData(
	APlayerController* OwnerPC,
	FGameplayDebuggerCanvasContext& CanvasContext)
{
	{ // Draw the sub-category bindings inline with the category header
		CanvasContext.CursorX += 200.f;
		CanvasContext.CursorY -= CanvasContext.GetLineHeight();
		const TCHAR* Active = TEXT("{green}");
		const TCHAR* Inactive = TEXT("{grey}");

		CanvasContext.Printf(TEXT("Items [%s%s{white}]\tSlots [%s%s{white}]"),
			bShowItems ? Active : Inactive, *GetInputHandlerDescription(0),
			bShowSlots ? Active : Inactive, *GetInputHandlerDescription(1));

		const bool bConsiderNetworkStatus = !OwnerPC->IsNetMode(NM_Standalone);
		if (bConsiderNetworkStatus)
		{
			CanvasContext.CursorY += CanvasContext.GetLineHeight();
			CanvasContext.Printf(TEXT("Network Filter {%s}(%s){white} [%s]"),
				*GetNetFilterColor(NetworkFiler), *GetNetFilterString(NetworkFiler), *GetInputHandlerDescription(2));
		}
	}

	if (LastDrawDataEndSize <= 0.f)
	{
		LastDrawDataEndSize = CanvasContext.Canvas->SizeY - CanvasContext.CursorY - CanvasContext.CursorX;
	}

	const float ThisDrawDataStartPos = CanvasContext.CursorY;

	constexpr FLinearColor BGColor(.1f, .1f, .1f, .8f);
	const FVector2D BGPos{ CanvasContext.CursorX, CanvasContext.CursorY };
	const FVector2D BGSize{ CanvasContext.Canvas->SizeX - (2.f * CanvasContext.CursorX), LastDrawDataEndSize };

	// Draw background box for better contrast
	FCanvasTileItem BG(FVector2d(0.f), BGSize, BGColor);
	BG.BlendMode = SE_BLEND_Translucent;
	CanvasContext.DrawItem(BG, BGPos.X, BGPos.Y);

	if (bShowItems)
	{
		DrawInventoryItems(CanvasContext, OwnerPC);
	}

	if (bShowSlots)
	{
		DrawInventorySlots(CanvasContext, OwnerPC);
	}

	LastDrawDataEndSize = CanvasContext.CursorY - ThisDrawDataStartPos;
}

void FGameplayDebuggerCategory_Itemization::DrawInventoryItems(
	FGameplayDebuggerCanvasContext& CanvasContext,
	const APlayerController* OwnerPC) const
{
	struct FDebugItemData
	{
		FRepData::FItem LocalItem;
		FRepData::FItem ServerItem;

		ENetworkStatus NetworkStatus = ENetworkStatus::LocalOnly;

		FString GetInstanceName(ENetworkStatus Filter) const
		{
			switch (Filter)
			{
			case ENetworkStatus::LocalOnly:
				{
					return FString::Printf(TEXT("{%s}%s"), *GetNetFilterColor(Filter), *LocalItem.InstanceName);
				}

			case ENetworkStatus::ServerOnly:
				{
					return FString::Printf(TEXT("{%s}%s"), *GetNetFilterColor(Filter), *ServerItem.InstanceName);
				}

			default:
				{
					if (LocalItem.InstanceName != ServerItem.InstanceName)
					{
						return FString::Printf(TEXT("{%s}%s {%s} %s"),
							*GetNetFilterColor(ENetworkStatus::ServerOnly), *ServerItem.InstanceName,
							*GetNetFilterColor(ENetworkStatus::LocalOnly), *LocalItem.InstanceName);
					}

					return FString::Printf(TEXT("{%s}%s"),
						*GetNetFilterColor(ENetworkStatus::Both), *LocalItem.InstanceName);
				}
			}
		}

		FString GetStackSize(ENetworkStatus Filter) const
		{
			const FString LocalStackSize = (LocalItem.MaxStackSize == 1)
				? FString::Printf(TEXT("{%s}%d"), *GetNetFilterColor(ENetworkStatus::LocalOnly), LocalItem.StackSize)
				: FString::Printf(TEXT("{%s}%d/{%s}%d"),
					*GetNetFilterColor(ENetworkStatus::LocalOnly), LocalItem.StackSize,
					*GetNetFilterColor(ENetworkStatus::LocalOnly), LocalItem.MaxStackSize);

			const FString ServerStackSize = (ServerItem.MaxStackSize == 1)
				? FString::Printf(TEXT("{%s}%d"), *GetNetFilterColor(ENetworkStatus::ServerOnly), ServerItem.StackSize)
				: FString::Printf(TEXT("{%s}%d/{%s}%d"),
					*GetNetFilterColor(ENetworkStatus::ServerOnly), ServerItem.StackSize,
					*GetNetFilterColor(ENetworkStatus::ServerOnly), ServerItem.MaxStackSize);

			switch (Filter)
			{
			case ENetworkStatus::LocalOnly:
				{
					return LocalStackSize;
				}
			case ENetworkStatus::ServerOnly:
				{
					return ServerStackSize;
				}

			default:
				{
					if (LocalItem.StackSize != ServerItem.StackSize)
					{

						return FString::Printf(TEXT("%s %s"), *ServerStackSize, *LocalStackSize);
					}

					return (ServerItem.MaxStackSize == 1)
						? FString::Printf(TEXT("{%s}%d"), *GetNetFilterColor(Filter), ServerItem.StackSize)
						: FString::Printf(TEXT("{%s}%d/{%s}%d"),
					*GetNetFilterColor(Filter), ServerItem.StackSize,
					*GetNetFilterColor(Filter), ServerItem.MaxStackSize);
				}
			}
		}

		FString GetSourceName(ENetworkStatus Filter) const
		{
			switch (Filter)
			{
			case ENetworkStatus::LocalOnly:
				{
					return FString::Printf(TEXT("{%s}%s"), *GetNetFilterColor(Filter), *LocalItem.SourceName);
				}
			case ENetworkStatus::ServerOnly:
				{
					return FString::Printf(TEXT("{%s}%s"), *GetNetFilterColor(Filter), *ServerItem.SourceName);
				}

			default:
				{
					if (LocalItem.SourceName != ServerItem.SourceName)
					{
						return FString::Printf(TEXT("{%s}%s {%s} %s"),
							*GetNetFilterColor(ENetworkStatus::ServerOnly), *ServerItem.SourceName,
							*GetNetFilterColor(ENetworkStatus::LocalOnly), *LocalItem.SourceName);
					}

					return FString::Printf(TEXT("{%s}%s"),
						*GetNetFilterColor(ENetworkStatus::Both), *LocalItem.SourceName);
				}
			}
		}
	};

	const bool bConsiderNetworkStatus = !OwnerPC->IsNetMode(NM_Standalone);
	const AInventoryBase* LocalInventory = bConsiderNetworkStatus ? UInventoryLibrary::FindInventory(FindLocalDebugActor()) : nullptr;
	const TArray<FRepData::FItem> LocalItems = LocalInventory ? CollectItems(OwnerPC, LocalInventory) : TArray<FRepData::FItem>();
	TArray<FRepData::FItem> ServerItems = DataPack.Items;

	TArray<FDebugItemData> ItemDebugData;
	if (NetworkFiler == ENetworkStatus::LocalOnly || NetworkFiler == ENetworkStatus::Both)
	{
		for (int32 Idx = LocalItems.Num() - 1; Idx >= 0; --Idx)
		{
			const auto& LocalItem = LocalItems[Idx];
			FDebugItemData& DebugDatum = ItemDebugData.Add_GetRef(
				FDebugItemData{
					.LocalItem = LocalItem,
					.NetworkStatus = ENetworkStatus::LocalOnly
				});

			if (NetworkFiler == ENetworkStatus::Both)
			{
				int32 ServerIndex = ServerItems.FindLastByPredicate([FindId = LocalItem.ItemId](const FRepData::FItem& Item) { return Item.ItemId == FindId; });
				if (ServerIndex != INDEX_NONE)
				{
					const auto& ServerItem = ServerItems[ServerIndex];
					DebugDatum.ServerItem = ServerItem;
					DebugDatum.NetworkStatus = ENetworkStatus::Both;

					// Remove from server-only array
					ServerItems.RemoveAtSwap(ServerIndex, EAllowShrinking::No);
				}
			}
		}
	}

	// Any other items count as server only
	if (NetworkFiler == ENetworkStatus::Both || NetworkFiler == ENetworkStatus::ServerOnly)
	{
		for (const auto& ServerItem : ServerItems)
		{
			auto& DebugDatum = ItemDebugData.AddDefaulted_GetRef();
			DebugDatum.ServerItem = ServerItem;
			DebugDatum.NetworkStatus = ENetworkStatus::ServerOnly;
		}
	}

	constexpr float Padding = 10.f;
	static float ObjNameSize = 0.f, SourceNameSize = 0.f, StackNameSize = 0.f, IdNameSize = 0.f, InstanceNameSize = 0.f;;
	if (ObjNameSize <= 0.f)
	{
		float TempSizeY = 0.f;
		CanvasContext.MeasureString(*LongestDebugObjectName, ObjNameSize, TempSizeY);
		CanvasContext.MeasureString(TEXT("source: MyDebugInventoryComponent_C"), SourceNameSize, TempSizeY);
		CanvasContext.MeasureString(TEXT("count: 000/000 000/000"), StackNameSize, TempSizeY);
		CanvasContext.MeasureString(TEXT("Id: 00000000"), IdNameSize, TempSizeY);
		CanvasContext.MeasureString(TEXT("instance: B_ItemInstanceBase_C_999"), InstanceNameSize, TempSizeY);

		ObjNameSize += Padding;
	}

	const float ColumnWidth = ObjNameSize * SourceNameSize + StackNameSize + IdNameSize + InstanceNameSize;
	const float CanvasWidth = CanvasContext.Canvas->SizeX;
	const int32 NumColumns = FMath::Max(1, FMath::FloorToInt(CanvasWidth / ColumnWidth));

	CanvasContext.Printf(TEXT("Items [%u]"), ItemDebugData.Num());
	CanvasContext.CursorX += 200.f;
	CanvasContext.CursorY -= CanvasContext.GetLineHeight();
	CanvasContext.Printf(TEXT("Legend: {%s}ServerOnly\t{%s}LocalOnly\t{%s}Both"),
		*GetNetFilterColor(ENetworkStatus::ServerOnly), *GetNetFilterColor(ENetworkStatus::LocalOnly), *GetNetFilterColor(ENetworkStatus::Both));

	CanvasContext.CursorX = 2.f * Padding;

	Algo::Sort(ItemDebugData, [](const FDebugItemData& A, const FDebugItemData& B)
	{
		return A.ServerItem.ItemId < B.ServerItem.ItemId || A.LocalItem.ItemId < B.LocalItem.ItemId;
	});
	for (const auto& Item : ItemDebugData)
	{
		const float CurX = CanvasContext.CursorX;
		const float CurY = CanvasContext.CursorY;

		float Indent = CurX;
		CanvasContext.PrintAt(Indent, CurY, Item.ServerItem.DisplayName.Left(35));
		Indent += ObjNameSize;

		CanvasContext.PrintAt(Indent, CurY, FString::Printf(TEXT("{grey}instance: %s"), *Item.GetInstanceName(NetworkFiler)));
		Indent += InstanceNameSize;

		CanvasContext.PrintAt(Indent, CurY, FString::Printf(TEXT("{grey}source: {white}%s"), *Item.GetSourceName(NetworkFiler)));
		Indent += SourceNameSize;

		CanvasContext.PrintAt(Indent, CurY, FString::Printf(TEXT("{grey}count: %s"), *Item.GetStackSize(NetworkFiler)));
		Indent += StackNameSize;

		CanvasContext.PrintAt(Indent, CurY, FString::Printf(TEXT("{grey}id: {white}%d"), Item.ServerItem.ItemId));

		// PrintAt would have reset these values, restore them.
		CanvasContext.CursorX = CurX + (CanvasWidth / NumColumns);
		CanvasContext.CursorY = CurY;

		// If overflowed, move to the next column
		if (CanvasContext.CursorX + CanvasWidth >= CanvasWidth)
		{
			CanvasContext.MoveToNewLine();
			CanvasContext.CursorX += Padding;
		}
	}

	// End row with a new line
	if (CanvasContext.CursorX != CanvasContext.DefaultX)
	{
		CanvasContext.MoveToNewLine();
	}
}

void FGameplayDebuggerCategory_Itemization::DrawInventorySlots(
	FGameplayDebuggerCanvasContext& CanvasContext,
	const APlayerController* OwnerPC) const
{
	struct FDebugSlotData
	{
		FRepData::FSlot LocalSlot;
		FRepData::FSlot ServerSlot;

		ENetworkStatus NetworkStatus = ENetworkStatus::LocalOnly;

		FString GetItemInSlot(ENetworkStatus Filter) const
		{
			const FString ServerName = ServerSlot.ItemName.IsEmpty() ? TEXT("empty") : ServerSlot.ItemName;
			const FString LocalName = LocalSlot.ItemName.IsEmpty() ? TEXT("empty") : LocalSlot.ItemName;

			switch (Filter)
			{
			case ENetworkStatus::LocalOnly:
				{
					return FString::Printf(TEXT("{%s}%s"), *GetNetFilterColor(Filter), *LocalName);
				}

			case ENetworkStatus::ServerOnly:
				{
					return FString::Printf(TEXT("{%s}%s"), *GetNetFilterColor(Filter), *ServerName);
				}

			default:
				{
					if (LocalName != ServerName)
					{
						return FString::Printf(TEXT("{%s}%s {%s} %s"),
							*GetNetFilterColor(ENetworkStatus::ServerOnly), *ServerName,
							*GetNetFilterColor(ENetworkStatus::LocalOnly), *LocalName);
					}

					return FString::Printf(TEXT("{%s}%s"),
						*GetNetFilterColor(ENetworkStatus::Both), *LocalName);
				}
			}
		}
	};

	const bool bConsiderNetworkStatus = !OwnerPC->IsNetMode(NM_Standalone);
	const AInventoryBase* LocalInventory = bConsiderNetworkStatus ? UInventoryLibrary::FindInventory(FindLocalDebugActor()) : nullptr;
	const TArray<FRepData::FSlot> LocalSlots = LocalInventory ? CollectSlots(OwnerPC, LocalInventory) : TArray<FRepData::FSlot>();
	TArray<FRepData::FSlot> ServerSlots = DataPack.Slots;

	TArray<FDebugSlotData> SlotDebugData;
	if (NetworkFiler == ENetworkStatus::LocalOnly || NetworkFiler == ENetworkStatus::Both)
	{
		for (int32 Idx = LocalSlots.Num() - 1; Idx >= 0; --Idx)
		{
			const auto& LocalSlot = LocalSlots[Idx];
			auto& DebugDatum = SlotDebugData.Add_GetRef(
				FDebugSlotData{
					.LocalSlot = LocalSlot,
					.NetworkStatus = ENetworkStatus::LocalOnly
				});

			if (NetworkFiler == ENetworkStatus::Both)
			{
				int32 ServerIndex = ServerSlots.FindLastByPredicate([FindCol = LocalSlot.ColumnIndex, FindRow = LocalSlot.RowIndex, FindGroup = LocalSlot.GroupName](const FRepData::FSlot& Slot)
			   {
				   return Slot.GroupName == FindGroup &&
					   Slot.RowIndex == FindRow &&
					   Slot.ColumnIndex == FindCol;
			   });

				if (ServerIndex != INDEX_NONE)
				{
					const auto& ServerSlot = ServerSlots[ServerIndex];
					DebugDatum.ServerSlot = ServerSlot;
					DebugDatum.NetworkStatus = ENetworkStatus::Both;

					// Remove from server-only array
					ServerSlots.RemoveAtSwap(ServerIndex, EAllowShrinking::No);
				}
			}
		}
	}

	// Any other items count as server only
	if (NetworkFiler == ENetworkStatus::Both || NetworkFiler == ENetworkStatus::ServerOnly)
	{
		for (const auto& ServerSLot : ServerSlots)
		{
			auto& DebugDatum = SlotDebugData.AddDefaulted_GetRef();
			DebugDatum.ServerSlot = ServerSLot;
			DebugDatum.NetworkStatus = ENetworkStatus::ServerOnly;
		}
	}



	constexpr float Padding = 10.f;
	CanvasContext.Printf(TEXT("Item Slots [%u]"), LocalSlots.Num());
	CanvasContext.MoveToNewLine();

	float IndexHeight, IndexWidth;
	CanvasContext.MeasureString("VeryVeryLongItemName", IndexWidth, IndexHeight);

	TMap<FString, TArray<FRepData::FSlot>> SlotGroupMap = {};

	struct FGridInfo
	{
		uint32 NumRows = 0;
		uint32 NumColumns = 0;
	};
	TMap<FString, FGridInfo> GridInfoMap = {};
	for (const auto& SlotData : DataPack.Slots)
	{
		TArray<FRepData::FSlot>& SlotList = SlotGroupMap.FindOrAdd(SlotData.GroupName);
		SlotList.Add(SlotData);
	}

	for (const auto& Pair : SlotGroupMap)
	{
		// Find the grid dimensions
		uint32 NumRows = 0;
		uint32 NumColumns = 0;
		for (const FRepData::FSlot& Slot : Pair.Value)
		{
			NumRows = FMath::Max(NumRows, Slot.RowIndex);
			NumColumns = FMath::Max(NumColumns, Slot.ColumnIndex);
		}

		GridInfoMap.Add(Pair.Key, FGridInfo(NumRows, NumColumns));
	}


	// Draw each group
	for (const auto& Pair : SlotGroupMap)
	{
		FGridInfo GridInfo = GridInfoMap.FindChecked(Pair.Key);

		//CanvasContext.CursorX += (IndexWidth * GridInfo.NumColumns) * 0.5f;
		CanvasContext.Printf(TEXT("Group [{lightblue}%s{white}]\t\tRows: %u, Columns: %u"), *Pair.Key, GridInfo.NumRows + 1, GridInfo.NumColumns + 1);

		const float CurPosX = CanvasContext.CursorX;
		const float CurPosY = CanvasContext.CursorY;

		/*
		{ // Draw Columns
			float IndentAmount = 0.0f;
			for (uint32 ColIdx = 0; ColIdx < GridInfo.NumColumns; ColIdx++)
			{
				IndentAmount += IndexWidth;

				CanvasContext.CursorY = CurPosY;
				CanvasContext.CursorX += IndentAmount;

				CanvasContext.Printf(TEXT("%u"), ColIdx);
			}
		}

		{ // Draw Rows
			for (uint32 RowIdx = 0; RowIdx < GridInfo.NumRows; RowIdx++)
			{
				CanvasContext.Printf(TEXT("%u"), RowIdx);
			}
		}*/

		// Draw header
		{
			CanvasContext.CursorX = Padding;
			for (uint32 ColIdx = 0; ColIdx <= GridInfo.NumColumns; ColIdx++)
			{
				CanvasContext.CursorX += IndexWidth;
				CanvasContext.PrintfAt(CanvasContext.CursorX, CurPosY, TEXT("{green}%u"), ColIdx);
			}
			CanvasContext.MoveToNewLine();
		}

		{ // Draw slot boxes
			CanvasContext.CursorX = Padding;
			for (uint32 RowIdx = 0; RowIdx <= GridInfo.NumRows; RowIdx++)
			{
				float CurX = 0.f;
				float CurY = CanvasContext.CursorY;

				for (uint32 ColIdx = 0; ColIdx <= GridInfo.NumColumns; ColIdx++)
				{
					if (ColIdx == 0)
					{
						CanvasContext.PrintfAt(Padding, CurY, TEXT("{green}%u"), RowIdx);
					}

					CurX += IndexWidth;
					CanvasContext.CursorX = CurX;
					CanvasContext.CursorY = CurY;

					// Row 0 -> draw column indexes
					/*if (RowIdx == 0)
					{
						CanvasContext.Printf(TEXT("%u"), ColIdx);
						continue;
					}

					// Column 0 -> draw row indexes
					if (ColIdx == 0)
					{
						CanvasContext.Printf(TEXT("%u"), RowIdx);
						continue;
					}*/

					/*constexpr FLinearColor BGColor(.1f, .1f, .1f, .8f);
					const FVector2D BGPos{CanvasContext.CursorX, CanvasContext.CursorY};
					const FVector2D BGSize{IndexWidth, IndexHeight};

					// Draw background box for better contrast
					FCanvasTileItem BG(FVector2d(0.f), BGSize, BGColor);
					BG.BlendMode = SE_BLEND_Translucent;
					CanvasContext.DrawItem(BG, BGPos.X, BGPos.Y);*/

					if (const FDebugSlotData* Slot = SlotDebugData.FindByPredicate([RowIdx, ColIdx, Pair](const FDebugSlotData& Lhs)
					{
						return Lhs.ServerSlot.RowIndex == RowIdx &&
							Lhs.ServerSlot.ColumnIndex == ColIdx &&
								Lhs.ServerSlot.GroupName == Pair.Key;
					}))
					{
						FString ItemName = Slot->GetItemInSlot(NetworkFiler);
						CanvasContext.PrintfAt(CanvasContext.CursorX, CanvasContext.CursorY, TEXT("%s {grey}%d"),
							*ItemName, Slot->ServerSlot.ItemId);
					}
				}

				CanvasContext.CursorX = Padding;
				CanvasContext.CursorY += CanvasContext.GetLineHeight();
			}
		}

		//CanvasContext.MoveToNewLine();
	}
}

void FGameplayDebuggerCategory_Itemization::FRepData::Serialize(FArchive& Ar)
{
	Ar << NetOwner;
	Ar << InventoryActor;

	int32 NumItems = Items.Num();
	Ar << NumItems;
	if (Ar.IsLoading())
	{
		Items.SetNum(NumItems);
	}

	for (int32 Idx = 0; Idx < NumItems; ++Idx)
	{
		Ar << Items[Idx].DisplayName;
		Ar << Items[Idx].InstanceName;
		Ar << Items[Idx].SourceName;
		Ar << Items[Idx].StackSize;
		Ar << Items[Idx].MaxStackSize;
		Ar << Items[Idx].ItemId;
	}


	int32 NumSlots = Slots.Num();
	Ar << NumSlots;
	if (Ar.IsLoading())
	{
		Slots.SetNum(NumSlots);
	}

	for (int32 Idx = 0; Idx < NumSlots; ++Idx)
	{
		Ar << Slots[Idx].RowIndex;
		Ar << Slots[Idx].ColumnIndex;
		Ar << Slots[Idx].ItemName;
		Ar << Slots[Idx].GroupName;
		Ar << Slots[Idx].ItemId;
	}
}

TArray<FGameplayDebuggerCategory_Itemization::FRepData::FItem> FGameplayDebuggerCategory_Itemization::CollectItems(
	const APlayerController* OwnerPC,
	const AInventoryBase* Inventory) const
{
	TArray<FRepData::FItem> RetVal;
	const auto& Items = Inventory->GetInventoryList();
	for (int32 Idx = 0; Idx < Items.Num(); ++Idx)
	{
		const auto& Entry = Items[Idx];

		FRepData::FItem ItemData;
		ItemData.DisplayName = Entry.GetItemDefinition()->GetItemName().ToString();
		ItemData.DisplayName.RemoveFromStart(DEFAULT_OBJECT_PREFIX);
		ItemData.DisplayName.RemoveFromEnd(TEXT("_C"));

		ItemData.InstanceName = GetNameSafe(Entry.GetItemInstance());
		ItemData.InstanceName.RemoveFromStart(DEFAULT_OBJECT_PREFIX);
		ItemData.InstanceName.RemoveFromEnd(TEXT("_C"));

		ItemData.SourceName = GetNameSafe(Entry.GetSourceObject());
		ItemData.SourceName.RemoveFromStart(DEFAULT_OBJECT_PREFIX);
		ItemData.SourceName.RemoveFromEnd(TEXT("_C"));

		ItemData.StackSize = Entry.GetStackSize();
		ItemData.MaxStackSize = Entry.GetItemDefinition()->GetMaxStackSize();

		ItemData.ItemId = Entry.GetItemId().Get();

		RetVal.Add(ItemData);
	}

	return MoveTemp(RetVal);
}

TArray<FGameplayDebuggerCategory_Itemization::FRepData::FSlot> FGameplayDebuggerCategory_Itemization::CollectSlots(
	const APlayerController* OwnerPC,
	const AInventoryBase* Inventory) const
{
	TArray<FRepData::FSlot> RetVal;

	const auto& Slots = Inventory->GetSlotList();
	for (int32 Idx = 0; Idx < Slots.Num(); ++Idx)
	{
		const auto& Slot = Slots[Idx];

		FRepData::FSlot SlotData;
		SlotData.GroupName = Slot.GetGroupTag().ToString();
		SlotData.RowIndex = Slot.GetRowIndex();
		SlotData.ColumnIndex = Slot.GetColumnIndex();
		SlotData.ItemName = TEXT("Empty");
		SlotData.ItemId = 0;

		if (const auto* ItemInSlot = Slot.GetItemEntryInSlot())
		{
			SlotData.ItemName = ItemInSlot->GetItemDefinition()->GetItemName().ToString();
			SlotData.ItemId = ItemInSlot->GetItemId().Get();
		}

		RetVal.Add(SlotData);
	}

	return MoveTemp(RetVal);
}

#endif
