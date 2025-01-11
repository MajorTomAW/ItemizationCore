// Fill out your copyright notice in the Description page of Project Settings.


#include "ItemizationCoreLog.h"


DEFINE_LOG_CATEGORY(LogInventorySystem);

#if WITH_EDITOR
extern ENGINE_API FString GPlayInEditorContextString;
#endif

FString Itemization::GetClientServerContextString(const UObject* Object)
{
	ENetRole Role = ROLE_None;

	if (const AActor* A = Cast<AActor>(Object))
	{
		Role = A->GetLocalRole();
	}
	else if (const UActorComponent* C = Cast<UActorComponent>(Object))
	{
		Role = C->GetOwnerRole();
	}

	if (Role != ROLE_None)
	{
		return (Role == ROLE_Authority) ? TEXT("[server]") : TEXT("[client]");
	}

#if WITH_EDITOR
	if (GIsEditor)
	{
		return FString::Printf(TEXT("[%s]"), *GPlayInEditorContextString);
	}
#endif

	return TEXT("[]");
}
