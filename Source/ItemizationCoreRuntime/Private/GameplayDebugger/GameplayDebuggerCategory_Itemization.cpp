// Copyright Epic Games, Inc. All Rights Reserved.


#include "GameplayDebuggerCategory_Itemization.h"

#include "ActorComponents/InventoryManager.h"
#include "Components/ItemComponentData_MaxStackSize.h"
#include "Engine/Canvas.h"

#if WITH_GAMEPLAY_DEBUGGER_MENU

const FString LongestDebugObjectName{ TEXT("ABCDEFGHIJKLMNOPQRSTUVWXYZ_ ABCDEFGH") };

FGameplayDebuggerCategory_Itemization::FGameplayDebuggerCategory_Itemization()
{
	SetDataPackReplication<FRepData>(&DataPack);
}

TSharedRef<FGameplayDebuggerCategory> FGameplayDebuggerCategory_Itemization::MakeInstance()
{
	return MakeShareable(new FGameplayDebuggerCategory_Itemization());
}

void FGameplayDebuggerCategory_Itemization::CollectData(APlayerController* OwnerPC, AActor* DebugActor)
{
	if (const UInventoryManager* InventoryManager = UInventoryManager::GetInventoryManager(DebugActor))
	{
		// Cache the package map for serialization
		const UNetConnection* NetConnection = OwnerPC->GetNetConnection();
		DataPack.ClientPackageMap = NetConnection ? NetConnection->PackageMap : nullptr;

		DataPack.Items.Reset();
		const TArray<FInventoryItemEntry>& ItemEntries = InventoryManager->GetInventoryItems();
		for (int32 Idx = 0; Idx < ItemEntries.Num(); Idx++)
		{
			const FInventoryItemEntry& ItemEntry = ItemEntries[Idx];
			FRepData::FInventoryItemDebug ItemData;

			ItemData.ItemName = GetNameSafe(ItemEntry.Definition);
			ItemData.ItemName.RemoveFromStart(DEFAULT_OBJECT_PREFIX);
			ItemData.ItemName.RemoveFromEnd(TEXT("_C"));

			ItemData.Source = GetNameSafe(ItemEntry.SourceObject.Get());
			ItemData.Source.RemoveFromStart(DEFAULT_OBJECT_PREFIX);

			ItemData.StackCount = ItemEntry.StackCount;

			if (const FItemComponentData_MaxStackSize* Data_MaxSize = ItemEntry.Definition->GetItemComponent<FItemComponentData_MaxStackSize>())
			{
				ItemData.MaxStackCount = Data_MaxSize->MaxStackSize.AsInteger();
			}
			else
			{
				ItemData.MaxStackCount = 1;
			}
			
			ItemData.Handle = ItemEntry.Handle.Get();

			ItemData.bIsActive = false;
			ItemData.bIsEquipped = false;

			DataPack.Items.Add(ItemData);
		}
	}
}

bool FGameplayDebuggerCategory_Itemization::WrapStringAccordingToViewport(const FString& InString, FString& OutString, FGameplayDebuggerCanvasContext& CanvasContext, float ViewportWidth) const
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

void FGameplayDebuggerCategory_Itemization::FRepData::Serialize(FArchive& Ar)
{
	int32 NumItems = Items.Num();
	Ar << NumItems;
	if (Ar.IsLoading())
	{
		Items.SetNum(NumItems);
	}

	for (int32 Idx = 0; Idx < NumItems; Idx++)
	{
		Ar << Items[Idx].ItemName;
		Ar << Items[Idx].Source;
		Ar << Items[Idx].StackCount;
		Ar << Items[Idx].MaxStackCount;
		Ar << Items[Idx].Handle;
		Ar << Items[Idx].bIsEquipped;
		Ar << Items[Idx].bIsActive;
	}
}

void FGameplayDebuggerCategory_Itemization::DrawData(APlayerController* OwnerPC, FGameplayDebuggerCanvasContext& CanvasContext)
{
	{ // Draw the Sub-Category bindings inline with the category header
		CanvasContext.CursorX += 200.f;
		CanvasContext.CursorY -= CanvasContext.GetLineHeight();
		const TCHAR* Active = TEXT("{green}");
		const TCHAR* Inactive = TEXT("{grey}");
		CanvasContext.Printf(TEXT("Items [%s%s{white}]"),
			Active, TEXT("Monitoring"));
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

	LastDrawDataEndSize = CanvasContext.CursorY - ThisDrawDataStartPos;
}

void FGameplayDebuggerCategory_Itemization::DrawInventoryItems(FGameplayDebuggerCanvasContext& CanvasContext, const APlayerController* OwnerPC) const
{
	const float CanvasWidth = CanvasContext.Canvas->SizeX;
	Algo::Sort(DataPack.Items, [](const FRepData::FInventoryItemDebug& A, const FRepData::FInventoryItemDebug& B)->bool
	{
		return A.Handle < B.Handle;
	});

	int32 NumEquipped = 0, NumActive = 0;
	for (const FRepData::FInventoryItemDebug& Item : DataPack.Items)
	{
		NumEquipped += Item.bIsEquipped;
		NumActive += Item.bIsActive;
	}

	constexpr float Padding = 10.f;
	static float ObjNameSize = 0.f, SourceNameSize = 0.f, StackNameSize = 0.f, HandleNameSize = 0.f;
	if (ObjNameSize <= 0.f)
	{
		float TempSizeY = 0.f;

		CanvasContext.MeasureString(*LongestDebugObjectName, ObjNameSize, TempSizeY);
		CanvasContext.MeasureString(TEXT("source: "), SourceNameSize, TempSizeY);
		CanvasContext.MeasureString(TEXT("stack: 00"), StackNameSize, TempSizeY);
		CanvasContext.MeasureString(TEXT("handle: 000"), HandleNameSize, TempSizeY);
		ObjNameSize += Padding;
	}

	const float ColumnWidth = ObjNameSize * 2 + SourceNameSize + StackNameSize + HandleNameSize;
	const int NumColumns = FMath::Max(1, FMath::FloorToInt(CanvasWidth / ColumnWidth));

	CanvasContext.Printf(TEXT("Inventory Items:"));
	CanvasContext.CursorX += 200.f;
	CanvasContext.CursorY -= CanvasContext.GetLineHeight();
	CanvasContext.Printf(TEXT("Legend:	{yellow}Granted [%d]	{cyan}Equipped [%d]		{green}Active [%d]"), DataPack.Items.Num(), NumEquipped, NumActive);
	CanvasContext.CursorX += Padding;

	for (const FRepData::FInventoryItemDebug& Item : DataPack.Items)
	{
		const float CurX = CanvasContext.CursorX;
		const float CurY = CanvasContext.CursorY;

		// Print positions manually to align them properly
		CanvasContext.PrintAt(CurX + ObjNameSize * 0, CurY, Item.bIsEquipped ? FColor::Cyan : FColor::Yellow, Item.ItemName.Left(35));
		CanvasContext.PrintAt(CurX + ObjNameSize * 0.5f, CurY, FString::Printf(TEXT("{grey}source: {white}%s"), *Item.Source));
		CanvasContext.PrintAt(CurX + ObjNameSize * 1.f + SourceNameSize, CurY, FString::Printf(TEXT("{grey}count: {white}%02d/{grey}%02d"), Item.StackCount, Item.MaxStackCount));
		CanvasContext.PrintAt(CurX + ObjNameSize * 1.5f + SourceNameSize + StackNameSize, CurY, FString::Printf(TEXT("{grey}handle: {white}%03d"), Item.Handle));

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

#endif
