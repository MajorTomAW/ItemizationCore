// Author: Tom Werner (dc: majort), 2026

#pragma once

#if WITH_GAMEPLAY_DEBUGGER_MENU
#include "GameplayDebuggerCategory.h"

class AInventoryBase;
class AActor;
class AController;
class UPackage;
class UPackageMap;

class FGameplayDebuggerCategory_Itemization : public FGameplayDebuggerCategory
{
	using ThisClass = FGameplayDebuggerCategory_Itemization;

public:
	FGameplayDebuggerCategory_Itemization();
	static TSharedRef<FGameplayDebuggerCategory> MakeInstance();

	//~ Begin FGameplayDebuggerCategory Interface
	virtual void CollectData(APlayerController* OwnerPC, AActor* DebugActor) override;
	virtual void DrawData(APlayerController* OwnerPC, FGameplayDebuggerCanvasContext& CanvasContext) override;
	//~ End FGameplayDebuggerCategory Interface

	/** Helping to see if items arent replicated for a reason. */
	enum class ENetworkStatus : uint8
	{
		ServerOnly, LocalOnly, Both, MAX
	};
	friend constexpr int32 operator+(const ENetworkStatus& Value) { return static_cast<int32>(Value); }
	static const FString NetFilterStrings[(int32)ENetworkStatus::MAX];
	static const FString NetFilterColors[(int32)ENetworkStatus::MAX];

	static FString GetNetFilterString(ENetworkStatus Value)
	{
		return NetFilterStrings[(int32)Value];
	}

	static FString GetNetFilterColor(ENetworkStatus Value)
	{
		return NetFilterColors[(int32)Value];
	}

protected:
	void DrawInventoryItems(FGameplayDebuggerCanvasContext& CanvasContext, const APlayerController* OwnerPC) const;
	void DrawInventorySlots(FGameplayDebuggerCanvasContext& CanvasContext, const APlayerController* OwnerPC) const;

	void ToggleShowItems() { bShowItems = !bShowItems; }
	void ToggleShowSlots() { bShowSlots = !bShowSlots; }
	void ToggleNetworkFiler() { NetworkFiler = (ENetworkStatus)(((int32)NetworkFiler + 1) % (int32)ENetworkStatus::MAX); }

protected:
	/** Replicated debug data. */
	struct FRepData
	{
		// Helper package for serialization
		TWeakObjectPtr<UPackageMap> ClientPackageMap;

		FString NetOwner, InventoryActor;

		struct FItem
		{
			FString DisplayName;
			FString InstanceName;
			FString SourceName;
			int32 StackSize = 0;
			int32 MaxStackSize = 0;
			uint32 ItemId = 0;
		};
		TArray<FItem> Items;

		struct FSlot
		{
			uint32 RowIndex = 0;
			uint32 ColumnIndex = 0;
			FString ItemName;
			FString GroupName;
			uint32 ItemId = 0;
		};
		TArray<FSlot> Slots;


		void Serialize(FArchive& Ar);
	} DataPack;

	TArray<FRepData::FItem> CollectItems(const APlayerController* OwnerPC, const AInventoryBase* Inventory) const;
	TArray<FRepData::FSlot> CollectSlots(const APlayerController* OwnerPC, const AInventoryBase* Inventory) const;

private:
	float LastDrawDataEndSize = 0.0f;

	bool bShowItems = true;
	bool bShowSlots = true;
	ENetworkStatus NetworkFiler = ENetworkStatus::Both;
};

#endif
