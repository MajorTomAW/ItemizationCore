// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#if WITH_GAMEPLAY_DEBUGGER_MENU
#include "GameplayDebuggerCategory.h"

class FGameplayDebuggerCategory_Itemization : public FGameplayDebuggerCategory
{
public:
	FGameplayDebuggerCategory_Itemization();
	static TSharedRef<FGameplayDebuggerCategory> MakeInstance();

	//~ Begin FGameplayDebuggerCategory Interface
	virtual void CollectData(APlayerController* OwnerPC, AActor* DebugActor) override;
	virtual void DrawData(APlayerController* OwnerPC, FGameplayDebuggerCanvasContext& CanvasContext) override;
	//~ End FGameplayDebuggerCategory Interface

	void OnShowItemHandlesToggle();
	void OnShowItemStatesToggle();
	void OnShowInstanceToggle();

protected:
	bool WrapStringAccordingToViewport(const FString& InString, FString& OutString, FGameplayDebuggerCanvasContext& CanvasContext, float ViewportWidth) const;

	void DrawInventoryItems(FGameplayDebuggerCanvasContext& CanvasContext, const APlayerController* OwnerPC) const;

protected:

	// Replicate-able debug data
	struct FRepData
	{
		// Helper package for serialization
		TWeakObjectPtr<UPackageMap> ClientPackageMap;

		struct FInventoryEquipmentDebug
		{
			FString InstanceName;
			FString Source;

			bool HasAnyData() const
			{
				return !InstanceName.IsEmpty() || !Source.IsEmpty(); 
			}
		};

		struct FInventoryItemDebug
		{
			FString ItemName;
			FString InstanceName;
			FString Source;
			int32 StackCount;
			int32 MaxStackCount;
			int32 Handle;
			bool bIsEquipped = false;
			bool bIsActive = false;

			FInventoryEquipmentDebug Equipment;
		};
		TArray<FInventoryItemDebug> Items;

		struct FInventorySystemDebug
		{
			FString OwnerActor;
			FString AvatarActor;
			int32 NumReplicatedItems = 0;
		};
		FInventorySystemDebug InventorySystem;

		void Serialize(FArchive& Ar);
	};
	FRepData DataPack;

private:
	float LastDrawDataEndSize = 0.f;

	bool bShowItemHandles = false;
	bool bShowItemStates = false;
	bool bShowInstance = true;
};
#endif