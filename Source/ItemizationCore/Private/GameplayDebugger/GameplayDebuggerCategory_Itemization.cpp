// Author: Tom Werner (MajorT), 2025


#include "GameplayDebuggerCategory_Itemization.h"

#include "Engine/Canvas.h"
#include "Inventory/IInventoryOwnerInterface.h"
#include "Inventory/InventoryBase.h"
#include "Items/InventoryItemInstance.h"
#include "Items/ItemDefinitionBase.h"

#if WITH_GAMEPLAY_DEBUGGER_MENU

const FString LongestDebugObjectName{ TEXT("ABCDEFGHIJKLMNOPQRSTUVWXYZ_ ABCDEFGH") };

FGameplayDebuggerCategory_Itemization::FGameplayDebuggerCategory_Itemization()
{
	SetDataPackReplication<FRepData>(&DataPack);

	// Hard coding these to avoid needing to import InputCore just for EKeys::GetFName().
	const FName NAME_KeyOne{ "One" };
	const FName NAME_KeyTwo{ "Two" };
	const FName NAME_KeyThree{ "Three" };
	const FName NAME_KeyFour{ "Four" };
	const FName NAME_KeyFive{ "Five" };

	typedef FGameplayDebuggerCategory_Itemization ThisClass;

	BindKeyPress(NAME_KeyOne, FGameplayDebuggerInputModifier::Shift, this, &ThisClass::OnShowItemHandlesToggle, EGameplayDebuggerInputMode::Local);
	BindKeyPress(NAME_KeyTwo, FGameplayDebuggerInputModifier::Shift, this, &ThisClass::OnShowItemSlotsToggle, EGameplayDebuggerInputMode::Local);
	BindKeyPress(NAME_KeyThree, FGameplayDebuggerInputModifier::Shift, this, &ThisClass::OnShowItemStatesToggle, EGameplayDebuggerInputMode::Local);
	BindKeyPress(NAME_KeyFour, FGameplayDebuggerInputModifier::Shift, this, &ThisClass::OnShowInstanceToggle, EGameplayDebuggerInputMode::Local);
	BindKeyPress(NAME_KeyFive, FGameplayDebuggerInputModifier::Shift, this, &ThisClass::OnShowInventoryOpsToggle, EGameplayDebuggerInputMode::Local);
}


void FGameplayDebuggerCategory_Itemization::FRepData::Serialize(FArchive& Ar)
{
	Ar << Inventory.OwnerActor;
	Ar << Inventory.NumReplicatedItems;
	Ar << Inventory.NumNonReplicatedItems;

	int32 NumItems = Items.Num();
	Ar << NumItems;
	if (Ar.IsLoading())
	{
		Items.SetNum(NumItems);
	}
	for (int32 i = 0; i < NumItems; i++)
	{
		Ar << Items[i].DisplayName;
		Ar << Items[i].InstanceName;
		Ar << Items[i].SourceObject;
		Ar << Items[i].StackSize;
		Ar << Items[i].MaxStackSize;
		Ar << Items[i].UID;
		Ar << Items[i].bIsEquipped;
		Ar << Items[i].bIsActive;
	}

	int32 NumOps = Operations.Num();
	Ar << NumOps;
	if (Ar.IsLoading())
	{
		Operations.SetNum(NumOps);
	}
	for (int32 i = 0; i < NumOps; i++)
	{
		Ar << Operations[i].DebugString;
		Ar << Operations[i].TimeRemaining;
	}

	int32 NumSlots = Slots.Num();
	Ar << NumSlots;
	if (Ar.IsLoading())
	{
		Slots.SetNum(NumSlots);
	}
	for (int32 i = 0; i < NumSlots; i++)
	{
		Ar << Slots[i].RowIndex;
		Ar << Slots[i].ColumnIndex;
		Ar << Slots[i].ItemName;
		Ar << Slots[i].GroupName;
	}
}

TSharedRef<FGameplayDebuggerCategory> FGameplayDebuggerCategory_Itemization::MakeInstance()
{
	return MakeShared<FGameplayDebuggerCategory_Itemization>();
}

void FGameplayDebuggerCategory_Itemization::CollectData(APlayerController* OwnerPC, AActor* DebugActor)
{
	const UActorComponent* InventoryComponent =
		DebugActor->FindComponentByInterface(UInventoryOwnerInterface::StaticClass());

	if (!IsValid(InventoryComponent))
	{
		const APawn* Pawn = Cast<APawn>(DebugActor);
		if (Pawn && Pawn->GetController())
		{
			InventoryComponent = Pawn->GetController()->FindComponentByInterface(UInventoryOwnerInterface::StaticClass());
		}
	}

	if (IsValid(InventoryComponent))
	{
		const IInventoryOwnerInterface* InventoryOwner = CastChecked<IInventoryOwnerInterface>(InventoryComponent);
		if (!ensure(InventoryOwner != nullptr))
		{
			return;
		}

		const AInventoryBase* Inventory = InventoryOwner->GetInventory();
		if (!ensure(Inventory))
		{
			return;
		}

		// Cache the package map for serialization
		const UNetConnection* NetConnection = OwnerPC->GetNetConnection();
		DataPack.ClientPackageMap = NetConnection ? NetConnection->PackageMap : nullptr;

		// Find inventory system data
		DataPack.Items.Reset();
		DataPack.Inventory.OwnerActor = GetNameSafe(Inventory->GetOwner());
		DataPack.Inventory.NumReplicatedItems = Inventory->GetAllItemInstances().Num();

		int32 NumNonReplicatedItems = 0;
		for (const FInventoryItemEntry& Item : Inventory->GetInventoryList())
		{
			if (Item.GetItemInstance() ? Item.GetItemInstance()->GetIsReplicated() : true)
			{
				continue;
			}

			NumNonReplicatedItems++;
		}
		DataPack.Inventory.NumNonReplicatedItems = NumNonReplicatedItems;

		// Find operations data
		const double SecondsNow = FPlatformTime::Seconds();
		constexpr float MaxOpLifetime = 10.f; //@TODO: Hardcoded for now, should be using the CVar defined in InventoryBase.cpp
		for (const TSharedPtr<FInventoryOp>& Op : Inventory->GetPendingOperations())
		{
			FRepData::FInventoryOpDebug OpData;
			OpData.DebugString = FString::Printf(TEXT("{yellow}%s    {white}%s"), *Op->OpName, *Op->OpDebugString);;
			OpData.TimeRemaining = FMath::Max(0.f, MaxOpLifetime - (SecondsNow - Op->ConstructionTime));

			DataPack.Operations.Add(OpData);
		}

		// Find items data
		const FInventoryItemContainer& ItemContainer = Inventory->GetInventoryList();
		for (int32 i = 0; i < ItemContainer.Num(); i++)
		{
			const FInventoryItemEntry& ItemEntry = ItemContainer[i];
			FRepData::FItemDebug ItemData;

			ItemData.DisplayName = ItemEntry.GetItemDefinition()->GetItemName().ToString();
			ItemData.DisplayName.RemoveFromStart(DEFAULT_OBJECT_PREFIX);
			ItemData.DisplayName.RemoveFromEnd(TEXT("_C"));

			ItemData.InstanceName = GetNameSafe(ItemEntry.GetItemInstance().GetObject());
			ItemData.InstanceName.RemoveFromStart(DEFAULT_OBJECT_PREFIX);
			ItemData.InstanceName.RemoveFromEnd(TEXT("_C"));

			ItemData.SourceObject = GetNameSafe(ItemEntry.GetSourceObject());
			ItemData.SourceObject.RemoveFromStart(DEFAULT_OBJECT_PREFIX);
			ItemData.SourceObject.RemoveFromEnd(TEXT("_C"));

			ItemData.StackSize = ItemEntry.GetStatValue(Itemization::Tags::TAG_ItemStat_CurrentStackSize);
			ItemData.MaxStackSize = ItemEntry.GetStatValue(Itemization::Tags::TAG_ItemStat_MaxStackSize);

			ItemData.UID = ItemEntry.GetItemHandle().Get();

			ItemData.bIsActive = false;
			ItemData.bIsEquipped = false;

			DataPack.Items.Add(ItemData);
		}

		// Find slots data
		for (const auto& GroupTag : Inventory->GetSlotList().GetAllItemGroups())
		{
			TArray<FInventoryItemSlot*> Slots = Inventory->GetSlotList().FindSlotsInGroup(GroupTag);
			for (const FInventoryItemSlot* Slot : Slots)
			{
				FRepData::FSlotDebug SlotData;
				SlotData.GroupName = GroupTag.ToString();
				SlotData.RowIndex = Slot->GetRowIndex();
				SlotData.ColumnIndex = Slot->GetColumnIndex();
				SlotData.ItemName = TEXT("Empty");

				if (const FInventoryItemEntry* ItemInSlot = ItemContainer.FindItemEntryByHandle(Slot->GetItemHandle()))
				{
					if (const UItemDefinitionBase* ItemDefinition = ItemInSlot->GetItemDefinition())
					{
						SlotData.ItemName = ItemDefinition->GetItemName().ToString();
					}
				}

				DataPack.Slots.Add(SlotData);
			}
		}
	}
}

void FGameplayDebuggerCategory_Itemization::DrawData(
	APlayerController* OwnerPC,
	FGameplayDebuggerCanvasContext& CanvasContext)
{
	{ // Draw the Sub-Category bindings inline with the category header
		CanvasContext.CursorX += 200.f;
		CanvasContext.CursorY -= CanvasContext.GetLineHeight();
		const TCHAR* Active = TEXT("{green}");
		const TCHAR* Inactive = TEXT("{grey}");

		CanvasContext.Printf(TEXT("Item UID [%s%s{white}]\tSlots [%s%s{white}]\tItem States [%s%s{white}]\tInstance [%s%s{white}]\tOperations [%s%s{white}]"),
			bShowItemHandles ? Active : Inactive, *GetInputHandlerDescription(0),
			bShowItemSlots ? Active : Inactive, *GetInputHandlerDescription(1),
			bShowItemStates ? Active : Inactive, *GetInputHandlerDescription(2),
			bShowInstance ? Active : Inactive, *GetInputHandlerDescription(3),
			bShowInventoryOps ? Active : Inactive, *GetInputHandlerDescription(4));
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

	DrawInventoryItems(CanvasContext, OwnerPC);
	DrawInventorySlots(CanvasContext, OwnerPC);

	LastDrawDataEndSize = CanvasContext.CursorY - ThisDrawDataStartPos;
}

bool FGameplayDebuggerCategory_Itemization::WrapStringAccordingToViewport(
	const FString& InString,
	FString& OutString,
	FGameplayDebuggerCanvasContext& CanvasContext,
	float ViewportWidth) const
{
	if (InString.IsEmpty())
	{
		return false;
	}

	// Clamp the width
	ViewportWidth = FMath::Max(ViewportWidth, 10.f);
	float StrWidth = 0.f, StrHeight = 0.f;

	CanvasContext.MeasureString(InString, StrWidth, StrHeight);

	int32 SubDivision = FMath::CeilToInt(StrWidth / ViewportWidth);
	OutString = InString;

	if (SubDivision > 1)
	{
		const int32 Step = OutString.Len() / SubDivision;

		// Subdivide if needed
		for (int32 i = SubDivision - 1; i > 0; --i)
		{
			OutString.InsertAt(i * Step -1, '\n');
		}

		return true;
	}

	return false;
}

void FGameplayDebuggerCategory_Itemization::DrawInventoryItems(
	FGameplayDebuggerCanvasContext& CanvasContext,
	const APlayerController* OwnerPC) const
{
	const float CanvasWidth = CanvasContext.Canvas->SizeX;
	Algo::Sort(DataPack.Items, [](const FRepData::FItemDebug& A, const FRepData::FItemDebug& B)->bool
	{
		return A.UID < B.UID;
	});

	int32 NumEquipped = 0, NumActive = 0, NumIdle = 0;
	NumIdle = DataPack.Items.Num();
	for (const FRepData::FItemDebug& Item : DataPack.Items)
	{
		NumEquipped += Item.bIsEquipped;
		NumActive += Item.bIsActive;
		NumIdle -= Item.bIsEquipped || Item.bIsActive;
	}

	constexpr float Padding = 10.f;
	static float ObjNameSize = 0.f, SourceNameSize = 0.f, StackNameSize = 0.f, HandleNameSize = 0.f;
	if (ObjNameSize <= 0.f)
	{
		float TempSizeY = 0.f;

		CanvasContext.MeasureString(*LongestDebugObjectName, ObjNameSize, TempSizeY);
		CanvasContext.MeasureString(TEXT("source: MyDebugInventoryComponent_C"), SourceNameSize, TempSizeY);
		CanvasContext.MeasureString(TEXT("stack: 00/00"), StackNameSize, TempSizeY);

		if (bShowItemHandles)
			CanvasContext.MeasureString(TEXT("UID: 000"), HandleNameSize, TempSizeY);

		if (bShowItemStates)
			CanvasContext.MeasureString(TEXT("state: active"), HandleNameSize, TempSizeY);

		if (bShowInstance)
			CanvasContext.MeasureString(TEXT("instance: 000000000"), HandleNameSize, TempSizeY);

		ObjNameSize += Padding;
	}

	const float ColumnWidth = ObjNameSize * 2 + SourceNameSize + StackNameSize + HandleNameSize;
	const int NumColumns = FMath::Max(1, FMath::FloorToInt(CanvasWidth / ColumnWidth));

	CanvasContext.Printf(TEXT("Inventory System"));
	CanvasContext.Printf(TEXT("Pending/Active Operations: {yellow}%d"), DataPack.Operations.Num());

	CanvasContext.CursorX += 200.f;

	CanvasContext.CursorY -= CanvasContext.GetLineHeight();
	CanvasContext.Printf(TEXT("Legend:	{yellow}Idle [%d]	{cyan}Equipped [%d]		{green}Active [%d]"), NumIdle, NumEquipped, NumActive);
	CanvasContext.Printf(TEXT("{white}Owner: {green}%s     {white}Num Non Replicated Instances: {green}%d		{white}Num Replicated Instances: {green}%d"),
		*DataPack.Inventory.OwnerActor, DataPack.Inventory.NumNonReplicatedItems, DataPack.Inventory.NumReplicatedItems);
	CanvasContext.CursorY += CanvasContext.GetLineHeight() * 2;

	CanvasContext.CursorX += Padding;

	if (bShowInventoryOps)
	{
		CanvasContext.CursorY -= CanvasContext.GetLineHeight();
		CanvasContext.CursorX = Padding;
		CanvasContext.Printf(TEXT("Inventory Operations"));

		CanvasContext.CursorX += 100.f;
		for (const auto& [DebugString, TimeRemaining] : DataPack.Operations)
		{
			CanvasContext.PrintAt(CanvasContext.CursorX, CanvasContext.CursorY, FString::Printf(
				TEXT("{green}%s {white}- Lifetime: {yellow}%.2fs"), *DebugString, TimeRemaining));

			//CanvasContext.Printf(TEXT("{green}%s {white}"), *DebugString);
			CanvasContext.CursorY += CanvasContext.GetLineHeight();
		}
		//CanvasContext.CursorY += CanvasContext.GetLineHeight() * 2;
	}

	CanvasContext.CursorX = Padding;
	CanvasContext.Printf(TEXT("Items"));
	CanvasContext.CursorX += Padding;

	for (const FRepData::FItemDebug& Item : DataPack.Items)
	{
		const float CurX = CanvasContext.CursorX;
		const float CurY = /*Item.Equipment.HasAnyData()
			? CanvasContext.CursorY + ( CanvasContext.GetLineHeight() * 1.5f )
			:*/ CanvasContext.CursorY;

		// Print positions manually to align them properly
		FColor ItemColor = Item.bIsEquipped ? FColor::Cyan : FColor::Yellow;
		ItemColor = Item.bIsActive ? FColor::Green : ItemColor;

		float IndentAmount = 0.f;
		CanvasContext.PrintAt(CurX + ObjNameSize * IndentAmount, CurY, ItemColor, Item.DisplayName.Left(35));
		IndentAmount += 0.4f;
		CanvasContext.PrintAt(CurX + ObjNameSize * IndentAmount, CurY, FString::Printf(TEXT("{grey}source: {white}%s"), *Item.SourceObject));
		IndentAmount += 0.4f;
		CanvasContext.PrintAt(CurX + ObjNameSize * IndentAmount + SourceNameSize, CurY, FString::Printf(TEXT("{grey}count: {white}%02d/{grey}%02d"), Item.StackSize, Item.MaxStackSize));
		IndentAmount += 0.4f;

		if (bShowItemHandles)
		{
			CanvasContext.PrintAt(CurX + ObjNameSize * IndentAmount + SourceNameSize + StackNameSize, CurY, FString::Printf(TEXT("{grey}UID: {white}%03d"), Item.UID));
			IndentAmount += 0.4f;
		}

		if (bShowItemStates)
		{
			FText ItemState = Item.bIsEquipped ? FText::FromString(TEXT("equipped")) : FText::FromString(TEXT("idle"));
			ItemState = Item.bIsActive ? FText::FromString(TEXT("active")) : ItemState;
			CanvasContext.PrintAt(CurX + ObjNameSize * IndentAmount + SourceNameSize + StackNameSize + HandleNameSize, CurY, FString::Printf(TEXT("{grey}state: {white}%s"), *ItemState.ToString()));
			IndentAmount += 0.4f;
		}

		if (bShowInstance)
		{
			CanvasContext.PrintAt(CurX + ObjNameSize * IndentAmount + SourceNameSize + StackNameSize + HandleNameSize, CurY, FString::Printf(TEXT("{grey}instance: {white}%s"), *Item.InstanceName));
			IndentAmount += 0.4f;
		}

		// Print Equipment stats
		/*if (Item.Equipment.HasAnyData())
		{
			CanvasContext.PrintAt(Padding * 4, CurY + CanvasContext.GetLineHeight(), FColor::White, TEXT("{grey}equipment"));
			CanvasContext.PrintAt(CurX + ObjNameSize * 0.4f, CurY + CanvasContext.GetLineHeight(), FColor::White,
				FString::Printf(TEXT("{grey}source: {white}%s"), *Item.Equipment.Source));

			CanvasContext.PrintAt(CurX + ObjNameSize * IndentAmount + SourceNameSize, CurY + CanvasContext.GetLineHeight(), FColor::White,
				FString::Printf(TEXT("{grey}instance: {white}%s"), *Item.Equipment.InstanceName));
		}*/

		// PrintAt would have reset these values, restore them.
		CanvasContext.CursorX = CurX + (CanvasWidth / NumColumns);
		CanvasContext.CursorY = CurY;

		// If we had any equipment data, make sure to move to the next line
		/*if (Item.Equipment.HasAnyData())
		{
			CanvasContext.MoveToNewLine();
			CanvasContext.CursorX += Padding;
		}*/

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
	if (!bShowItemSlots)
	{
		return;
	}

	constexpr float PaddingX = 10.f;

	CanvasContext.Printf(TEXT("Item Slots [%u]"), DataPack.Slots.Num());
	CanvasContext.MoveToNewLine();

	float IndexHeight, IndexWidth;
	CanvasContext.MeasureString("VeryVeryLongItemName", IndexWidth, IndexHeight);

	TMap<FString, TArray<FRepData::FSlotDebug>> SlotGroupMap;

	struct FGridInfo
	{
		uint32 NumRows = 0;
		uint32 NumColumns = 0;
	};
	TMap<FString, FGridInfo> GridInfoMap;
	for (const auto& SlotData : DataPack.Slots)
	{
		TArray<FRepData::FSlotDebug>& SlotList = SlotGroupMap.FindOrAdd(SlotData.GroupName);
		SlotList.Add(SlotData);
	}

	for (const auto& Pair : SlotGroupMap)
	{
		// Find the grid dimensions
		uint32 NumRows = 0;
		uint32 NumColumns = 0;
		for (const auto& Slot : Pair.Value)
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
			CanvasContext.CursorX = PaddingX;
			for (uint32 ColIdx = 0; ColIdx <= GridInfo.NumColumns; ColIdx++)
			{
				CanvasContext.CursorX += IndexWidth;
				CanvasContext.PrintfAt(CanvasContext.CursorX, CurPosY, TEXT("{yellow}%u"), ColIdx);
			}
			CanvasContext.MoveToNewLine();
		}

		{ // Draw slot boxes
			CanvasContext.CursorX = PaddingX;
			for (uint32 RowIdx = 0; RowIdx <= GridInfo.NumRows; RowIdx++)
			{
				float CurX = 0.f;
				float CurY = CanvasContext.CursorY;

				for (uint32 ColIdx = 0; ColIdx <= GridInfo.NumColumns; ColIdx++)
				{
					if (ColIdx == 0)
					{
						CanvasContext.PrintfAt(PaddingX, CurY, TEXT("{yellow}%u"), RowIdx);
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

					if (const FRepData::FSlotDebug* Slot = DataPack.Slots.FindByPredicate([RowIdx, ColIdx, Pair](const FRepData::FSlotDebug& Lhs)
					{
						return Lhs.RowIndex == RowIdx && Lhs.ColumnIndex == ColIdx && Lhs.GroupName == Pair.Key;
					}))
					{
						FString ItemName = Slot->ItemName;
						if (ItemName == "Empty")
						{
							CanvasContext.PrintfAt(CanvasContext.CursorX, CanvasContext.CursorY, TEXT("{grey}%s"), *Slot->ItemName);
						}
						else
						{
							CanvasContext.PrintfAt(CanvasContext.CursorX, CanvasContext.CursorY, TEXT("{white}%s"), *Slot->ItemName);
						}
					}
				}

				CanvasContext.CursorX = PaddingX;
				CanvasContext.CursorY += CanvasContext.GetLineHeight();
			}
		}

		//CanvasContext.MoveToNewLine();
	}

}


#endif
