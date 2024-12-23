// Copyright Epic Games, Inc. All Rights Reserved.

#include "Actions/AsyncAction_FindItemComponent.h"

#include "ItemDefinition.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "UObject/ScriptMacros.h"
#include "UObject/Stack.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AsyncAction_FindItemComponent)

UAsyncAction_FindItemComponent* UAsyncAction_FindItemComponent::FindItemComponent(UItemDefinition* ItemDefinition, UScriptStruct* ComponentType)
{
	UWorld* World = GEngine->GetWorldFromContextObject(ItemDefinition, EGetWorldErrorMode::LogAndReturnNull);
	if (World == nullptr)
	{
		return nullptr;
	}
	
	UAsyncAction_FindItemComponent* Action = NewObject<UAsyncAction_FindItemComponent>();
	Action->WorldPtr = World;
	Action->ItemPtr = ItemDefinition;
	Action->ComponentTypePtr = ComponentType;
	Action->RegisterWithGameInstance(World);

	return Action;
}

bool UAsyncAction_FindItemComponent::GetComponent(int32& OutComponent)
{
	checkNoEntry();
	return false;
}

void UAsyncAction_FindItemComponent::execGetComponent(UObject* Context, FFrame& Stack, void* const Z_Param__Result)
{
	Stack.MostRecentPropertyAddress = nullptr;
	Stack.StepCompiledIn<FStructProperty>(nullptr);
	void* ComponentPtr = Stack.MostRecentPropertyAddress;
	FStructProperty* ComponentProperty = CastField<FStructProperty>(Stack.MostRecentProperty);
	P_FINISH;

	bool bSuccess = false;

	if (
		(ComponentProperty != nullptr) &&
		(ComponentProperty->Struct != nullptr) &&
		(ComponentPtr != nullptr) &&
		(ComponentProperty->Struct == P_THIS->ComponentTypePtr.Get()) &&
		(P_THIS->FoundItemComponentPtr != nullptr)
		)
	{
		ComponentProperty->Struct->CopyScriptStruct(ComponentPtr, P_THIS->FoundItemComponentPtr);
		bSuccess = true;
	}

	*static_cast<bool*>(RESULT_PARAM) = bSuccess;
}

void UAsyncAction_FindItemComponent::Activate()
{
	Super::Activate();
}

void UAsyncAction_FindItemComponent::SetReadyToDestroy()
{
	Super::SetReadyToDestroy();
}
