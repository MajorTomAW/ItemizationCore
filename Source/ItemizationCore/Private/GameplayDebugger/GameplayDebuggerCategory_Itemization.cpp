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

	typedef FGameplayDebuggerCategory_Itemization ThisClass;

	BindKeyPress(NAME_KeyOne, FGameplayDebuggerInputModifier::Shift, this, &ThisClass::OnShowItemHandlesToggle, EGameplayDebuggerInputMode::Local);
	BindKeyPress(NAME_KeyTwo, FGameplayDebuggerInputModifier::Shift, this, &ThisClass::OnShowItemStatesToggle, EGameplayDebuggerInputMode::Local);
	BindKeyPress(NAME_KeyThree, FGameplayDebuggerInputModifier::Shift, this, &ThisClass::OnShowInstanceToggle, EGameplayDebuggerInputMode::Local);
	BindKeyPress(NAME_KeyFour, FGameplayDebuggerInputModifier::Shift, this, &ThisClass::OnShowInventoryOpsToggle, EGameplayDebuggerInputMode::Local);
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
			OpData.DisplayName = Op->OpName;
			OpData.TimeRemaining = FMath::Max(0.f, MaxOpLifetime - (SecondsNow - Op->ConstructionTime));

			DataPack.Operations.Add(OpData);
		}

		// Find items data
		const FInventoryItemContainer& ItemContainer = Inventory->GetInventoryList();
		for (int32 i = 0; i < ItemContainer.Num(); i++)
		{
			const FInventoryItemEntry& ItemEntry = ItemContainer[i];
			FRepData::FItemDebug ItemData;

			ItemData.DisplayName = ItemEntry.ItemDefinition->GetItemName().ToString();
			ItemData.DisplayName.RemoveFromStart(DEFAULT_OBJECT_PREFIX);
			ItemData.DisplayName.RemoveFromEnd(TEXT("_C"));

			ItemData.InstanceName = GetNameSafe(ItemEntry.GetItemInstance().GetObject());
			ItemData.InstanceName.RemoveFromStart(DEFAULT_OBJECT_PREFIX);
			ItemData.InstanceName.RemoveFromEnd(TEXT("_C"));

			ItemData.SourceObject = GetNameSafe(ItemEntry.SourceObject.Get());
			ItemData.SourceObject.RemoveFromStart(DEFAULT_OBJECT_PREFIX);
			ItemData.SourceObject.RemoveFromEnd(TEXT("_C"));

			ItemData.StackSize = ItemEntry.GetStatValue(Itemization::Tags::TAG_ItemStat_CurrentStackSize);
			ItemData.MaxStackSize = ItemEntry.GetStatValue(Itemization::Tags::TAG_ItemStat_MaxStackSize);

			ItemData.UID = ItemEntry.ItemHandle.Get();

			ItemData.bIsActive = false;
			ItemData.bIsEquipped = false;

			DataPack.Items.Add(ItemData);
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
		
		CanvasContext.Printf(TEXT("Item UID [%s%s{white}]\tItem States [%s%s{white}]\tInstance [%s%s{white}]\tOperations [%s%s{white}]"),
			bShowItemHandles ? Active : Inactive, *GetInputHandlerDescription(0),
			bShowItemStates ? Active : Inactive, *GetInputHandlerDescription(1),
			bShowInstance ? Active : Inactive, *GetInputHandlerDescription(2),
			bShowInventoryOps ? Active : Inactive, *GetInputHandlerDescription(3));
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
		CanvasContext.CursorY += CanvasContext.GetLineHeight() * 2;
		for (const auto& [DisplayName, TimeRemaining] : DataPack.Operations)
		{
			CanvasContext.Printf(TEXT("[green}%s {white}- Lifetime: {yellow}%.2fs"), *DisplayName, TimeRemaining);
		}
		CanvasContext.CursorY += CanvasContext.GetLineHeight() * 2;
	}

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


#endif