// Author: Tom Werner (MajorT), 2025

#pragma once

#include "CoreMinimal.h"

#if WITH_GAMEPLAY_DEBUGGER_MENU
#include "GameplayDebuggerCategory.h"

class AActor;
class AController;
class UPackage;
class UPackageMap;

class FGameplayDebuggerCategory_Itemization : public FGameplayDebuggerCategory
{
public:
	FGameplayDebuggerCategory_Itemization();
	static TSharedRef<FGameplayDebuggerCategory> MakeInstance();

	//~ Begin FGameplayDebuggerCategory Interface
	virtual void CollectData(APlayerController* OwnerPC, AActor* DebugActor) override;
	virtual void DrawData(APlayerController* OwnerPC, FGameplayDebuggerCanvasContext& CanvasContext) override;
	//~ End FGameplayDebuggerCategory Interface

protected:
	bool WrapStringAccordingToViewport(const FString& InString, FString& OutString, FGameplayDebuggerCanvasContext& CanvasContext, float ViewportWidth) const;
	void DrawInventoryItems(FGameplayDebuggerCanvasContext& CanvasContext, const APlayerController* OwnerPC) const;

	void OnShowItemHandlesToggle() { bShowItemHandles = !bShowItemHandles; }
	void OnShowItemStatesToggle() { bShowItemStates = !bShowItemStates; }
	void OnShowInstanceToggle() { bShowInstance = !bShowInstance; }
	void OnShowInventoryOpsToggle() { bShowInventoryOps = !bShowInventoryOps; }

protected:
	/** Replicated debug data. */
	struct FRepData
	{
		// Helper package for serialization
		TWeakObjectPtr<UPackageMap> ClientPackageMap;

		struct FItemDebug
		{
			FString DisplayName;
			FString InstanceName;
			FString SourceObject;
			int32 StackSize = 0;
			int32 MaxStackSize = 0;
			uint32 UID = 0;
			bool bIsEquipped = false;
			bool bIsActive = false;
		};
		TArray<FItemDebug> Items;

		struct FInventoryOpDebug
		{
			FString DisplayName;
			double TimeRemaining = 0.0;
		};
		TArray<FInventoryOpDebug> Operations;


		struct FInventorySystemDebug
		{
			FString OwnerActor;
			uint32 NumReplicatedItems = 0;
			uint32 NumNonReplicatedItems = 0;
		};
		FInventorySystemDebug Inventory;


		void Serialize(FArchive& Ar);
	};
	FRepData DataPack;

private:
	float LastDrawDataEndSize = 0.f;

	bool bShowItemHandles = false;
	bool bShowItemStates = false;
	bool bShowInstance = true;
	bool bShowInventoryOps = false;
};

#endif