// Copyright Epic Games, Inc. All Rights Reserved.

#include "ItemizationCore.h"

#define LOCTEXT_NAMESPACE "FItemizationCoreModule"

#if WITH_GAMEPLAY_DEBUGGER
#include "GameplayDebugger.h"
#include "Debugging/GameplayDebuggerCategory_Itemization.h"
#endif

void FItemizationCoreModule::StartupModule()
{
#if WITH_GAMEPLAY_DEBUGGER
	IGameplayDebugger& GameplayDebugger = IGameplayDebugger::Get();
	GameplayDebugger.RegisterCategory(
		"Itemization",
		IGameplayDebugger::FOnGetCategory::CreateStatic(&FGameplayDebuggerCategory_Itemization::MakeInstance),
		EGameplayDebuggerCategoryState::EnabledInGame,
		3);
	GameplayDebugger.NotifyCategoriesChanged();
#endif
}

void FItemizationCoreModule::ShutdownModule()
{
#if WITH_GAMEPLAY_DEBUGGER
	if (IGameplayDebugger::IsAvailable())
	{
		IGameplayDebugger& GameplayDebugger = IGameplayDebugger::Get();
		GameplayDebugger.UnregisterCategory("Itemization");
		GameplayDebugger.NotifyCategoriesChanged();
	}
#endif
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FItemizationCoreModule, ItemizationCore)
