// Author: Tom Werner (dc: majort), 2026


#include "ItemizationCoreLogChannels.h"

DEFINE_LOG_CATEGORY(LogItemization)
DEFINE_LOG_CATEGORY(LogItemizationUI)

#if WITH_EDITOR
extern ENGINE_API FString GPlayInEditorContextString;
#endif

FString UE::ItemizationCore::GetNetContextString(const UObject* Obj)
{
	ENetRole Role = ROLE_None;

	if (const AActor* A = Cast<AActor>(Obj))
	{
		Role = A->GetLocalRole();
	}
	else if (const UActorComponent* AC = Cast<UActorComponent>(Obj))
	{
		Role = AC->GetOwnerRole();
	}
	else if (const AActor* Outer = Cast<AActor>(Obj->GetOuter()))
	{
		Role = Outer->GetLocalRole();
	}
	else if (const AActor* Outermost = Cast<AActor>(Obj->GetOutermostObject()))
	{
		Role = Outermost->GetLocalRole();
	}

	if (Role != ROLE_None)
	{
		return (Role == ROLE_Authority) ? TEXT("server") : TEXT("client");
	}
#if WITH_EDITOR
	if (GIsEditor)
	{
		return FString::Printf(TEXT("%s"), *GPlayInEditorContextString);
	}
#endif

	return TEXT("");
}
