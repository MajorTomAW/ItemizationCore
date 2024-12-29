// Copyright Epic Games, Inc. All Rights Reserved.


#include "GameplayDebuggerCategory_Itemization.h"

#include "ActorComponents/InventoryManager.h"
#include "Components/ItemComponentData_MaxStackSize.h"
#include "InventoryItemInstance.h"
#include "ActorComponents/EquipmentManager.h"
#include "Engine/Canvas.h"

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

	typedef FGameplayDebuggerCategory_Itemization ThisClass;

	BindKeyPress(NAME_KeyOne, FGameplayDebuggerInputModifier::Shift, this, &ThisClass::OnShowItemHandlesToggle, EGameplayDebuggerInputMode::Local);
	BindKeyPress(NAME_KeyTwo, FGameplayDebuggerInputModifier::Shift, this, &ThisClass::OnShowItemStatesToggle, EGameplayDebuggerInputMode::Local);
	BindKeyPress(NAME_KeyThree, FGameplayDebuggerInputModifier::Shift, this, &ThisClass::OnShowInstanceToggle, EGameplayDebuggerInputMode::Local);
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
		DataPack.InventorySystem.OwnerActor = GetNameSafe(InventoryManager->GetOwnerActor());
		DataPack.InventorySystem.AvatarActor = GetNameSafe(InventoryManager->GetAvatarActor());
		DataPack.InventorySystem.NumReplicatedItems = InventoryManager->GetReplicatedItemInstances().Num();
		
		const TArray<FInventoryItemEntry>& ItemEntries = InventoryManager->GetInventoryItems();
		for (int32 Idx = 0; Idx < ItemEntries.Num(); Idx++)
		{
			const FInventoryItemEntry& ItemEntry = ItemEntries[Idx];
			FRepData::FInventoryItemDebug ItemData;

			ItemData.ItemName = GetNameSafe(ItemEntry.Definition);
			ItemData.ItemName.RemoveFromStart(DEFAULT_OBJECT_PREFIX);
			ItemData.ItemName.RemoveFromEnd(TEXT("_C"));

			ItemData.InstanceName = GetNameSafe(ItemEntry.Instance);
			ItemData.InstanceName.RemoveFromStart(DEFAULT_OBJECT_PREFIX);
			ItemData.InstanceName.RemoveFromEnd(TEXT("_C"));

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

			if (const UEquipmentManager* EquipmentManager = UEquipmentManager::GetEquipmentManager(DebugActor))
			{
				if (const FInventoryEquipmentEntry* EquipmentEntry = EquipmentManager->FindEquipmentEntryFromHandle(ItemEntry.Handle))
				{
					ItemData.bIsEquipped = true;
					ItemData.Equipment.InstanceName = GetNameSafe(EquipmentEntry->Instance);
					ItemData.Equipment.Source = GetNameSafe(EquipmentEntry->SourceObject);
				}
			}

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
	Ar << InventorySystem.AvatarActor;
	Ar << InventorySystem.OwnerActor;
	Ar << InventorySystem.NumReplicatedItems;
	
	int32 NumItems = Items.Num();
	Ar << NumItems;
	if (Ar.IsLoading())
	{
		Items.SetNum(NumItems);
	}

	for (int32 Idx = 0; Idx < NumItems; Idx++)
	{
		Ar << Items[Idx].ItemName;
		Ar << Items[Idx].InstanceName;
		Ar << Items[Idx].Source;
		Ar << Items[Idx].StackCount;
		Ar << Items[Idx].MaxStackCount;
		Ar << Items[Idx].Handle;
		Ar << Items[Idx].bIsEquipped;
		Ar << Items[Idx].bIsActive;
		Ar << Items[Idx].Equipment.Source;
		Ar << Items[Idx].Equipment.InstanceName;
	}
}

void FGameplayDebuggerCategory_Itemization::DrawData(APlayerController* OwnerPC, FGameplayDebuggerCanvasContext& CanvasContext)
{
	{ // Draw the Sub-Category bindings inline with the category header
		CanvasContext.CursorX += 200.f;
		CanvasContext.CursorY -= CanvasContext.GetLineHeight();
		const TCHAR* Active = TEXT("{green}");
		const TCHAR* Inactive = TEXT("{grey}");
		
		CanvasContext.Printf(TEXT("Item Handles [%s%s{white}]\tItem States [%s%s{white}]\tInstance [%s%s{white}]"),
			bShowItemHandles ? Active : Inactive, *GetInputHandlerDescription(0),
			bShowItemStates ? Active : Inactive, *GetInputHandlerDescription(1),
			bShowInstance ? Active : Inactive, *GetInputHandlerDescription(2));
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

	
	FSlateRenderTransform(TQuat2<float>(45.f));

	LastDrawDataEndSize = CanvasContext.CursorY - ThisDrawDataStartPos;
}

void FGameplayDebuggerCategory_Itemization::OnShowItemHandlesToggle()
{
	bShowItemHandles = !bShowItemHandles;
}

void FGameplayDebuggerCategory_Itemization::OnShowItemStatesToggle()
{
	bShowItemStates = !bShowItemStates;
}

void FGameplayDebuggerCategory_Itemization::OnShowInstanceToggle()
{
	bShowInstance = !bShowInstance;
}

void FGameplayDebuggerCategory_Itemization::DrawInventoryItems(FGameplayDebuggerCanvasContext& CanvasContext, const APlayerController* OwnerPC) const
{
	const float CanvasWidth = CanvasContext.Canvas->SizeX;
	Algo::Sort(DataPack.Items, [](const FRepData::FInventoryItemDebug& A, const FRepData::FInventoryItemDebug& B)->bool
	{
		return A.Handle < B.Handle;
	});

	int32 NumEquipped = 0, NumActive = 0, NumIdle = 0;
	NumIdle = DataPack.Items.Num();
	for (const FRepData::FInventoryItemDebug& Item : DataPack.Items)
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
		CanvasContext.MeasureString(TEXT("source: "), SourceNameSize, TempSizeY);
		CanvasContext.MeasureString(TEXT("stack: 00/00"), StackNameSize, TempSizeY);

		if (bShowItemHandles)
			CanvasContext.MeasureString(TEXT("handle: 000"), HandleNameSize, TempSizeY);

		if (bShowItemStates)
			CanvasContext.MeasureString(TEXT("state: active"), HandleNameSize, TempSizeY);

		if (bShowInstance)
			CanvasContext.MeasureString(TEXT("instance: 000000000"), HandleNameSize, TempSizeY);
		
		ObjNameSize += Padding;
	}

	const float ColumnWidth = ObjNameSize * 2 + SourceNameSize + StackNameSize + HandleNameSize;
	const int NumColumns = FMath::Max(1, FMath::FloorToInt(CanvasWidth / ColumnWidth));

	CanvasContext.Printf(TEXT("Inventory System"));
	CanvasContext.CursorX += 200.f;
	CanvasContext.CursorY -= CanvasContext.GetLineHeight();
	CanvasContext.Printf(TEXT("Legend:	{yellow}Idle [%d]	{cyan}Equipped [%d]		{green}Active [%d]"), NumIdle, NumEquipped, NumActive);
	CanvasContext.Printf(TEXT("{white}Owner: {green}%s     {white}Avatar: {green}%s		{white}Num Replicated Instances: {green}%d"),
		*DataPack.InventorySystem.OwnerActor, *DataPack.InventorySystem.AvatarActor, DataPack.InventorySystem.NumReplicatedItems);
	CanvasContext.CursorY += CanvasContext.GetLineHeight();
	
	CanvasContext.CursorX += Padding;

	for (const FRepData::FInventoryItemDebug& Item : DataPack.Items)
	{
		const float CurX = CanvasContext.CursorX;
		const float CurY = Item.Equipment.HasAnyData()
			? CanvasContext.CursorY + ( CanvasContext.GetLineHeight() * 1.5f )
			: CanvasContext.CursorY;

		// Print positions manually to align them properly
		FColor ItemColor = Item.bIsEquipped ? FColor::Cyan : FColor::Yellow;
		ItemColor = Item.bIsActive ? FColor::Green : ItemColor;

		float IndentAmount = 0.f;
		CanvasContext.PrintAt(CurX + ObjNameSize * IndentAmount, CurY, ItemColor, Item.ItemName.Left(35));
		IndentAmount += 0.4f;
		CanvasContext.PrintAt(CurX + ObjNameSize * IndentAmount, CurY, FString::Printf(TEXT("{grey}source: {white}%s"), *Item.Source));
		IndentAmount += 0.4f;
		CanvasContext.PrintAt(CurX + ObjNameSize * IndentAmount + SourceNameSize, CurY, FString::Printf(TEXT("{grey}count: {white}%02d/{grey}%02d"), Item.StackCount, Item.MaxStackCount));
		IndentAmount += 0.4f;

		if (bShowItemHandles)
		{
			CanvasContext.PrintAt(CurX + ObjNameSize * IndentAmount + SourceNameSize + StackNameSize, CurY, FString::Printf(TEXT("{grey}handle: {white}%03d"), Item.Handle));
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
		if (Item.Equipment.HasAnyData())
		{
			CanvasContext.PrintAt(Padding * 4, CurY + CanvasContext.GetLineHeight(), FColor::White, TEXT("{grey}equipment"));
			CanvasContext.PrintAt(CurX + ObjNameSize * 0.4f, CurY + CanvasContext.GetLineHeight(), FColor::White,
				FString::Printf(TEXT("{grey}source: {white}%s"), *Item.Equipment.Source));
			
			CanvasContext.PrintAt(CurX + ObjNameSize * IndentAmount + SourceNameSize, CurY + CanvasContext.GetLineHeight(), FColor::White,
				FString::Printf(TEXT("{grey}instance: {white}%s"), *Item.Equipment.InstanceName));
		}

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
