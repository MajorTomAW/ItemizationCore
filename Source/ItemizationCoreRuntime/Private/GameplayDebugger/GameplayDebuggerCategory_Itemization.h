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

protected:
	bool WrapStringAccordingToViewport(const FString& InString, FString& OutString, FGameplayDebuggerCanvasContext& CanvasContext, float ViewportWidth) const;

	void DrawInventoryItems(FGameplayDebuggerCanvasContext& CanvasContext, const APlayerController* OwnerPC) const;

protected:

	// Replicate-able debug data
	struct FRepData
	{
		// Helper package for serialization
		TWeakObjectPtr<UPackageMap> ClientPackageMap;

		struct FInventoryItemDebug
		{
			FString ItemName;
			FString Source;
			int32 StackCount;
			int32 MaxStackCount;
			int32 Handle;
			bool bIsEquipped = false;
			bool bIsActive = false;
		};
		TArray<FInventoryItemDebug> Items;

		void Serialize(FArchive& Ar);
	};
	FRepData DataPack;

private:
	float LastDrawDataEndSize = 0.f;
};
#endif