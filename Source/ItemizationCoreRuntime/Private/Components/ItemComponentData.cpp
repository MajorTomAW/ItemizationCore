// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/ItemComponentData.h"

#if WITH_EDITOR
EDataValidationResult FItemComponentData::IsDataValid(class FDataValidationContext& Context) const
{
	return EDataValidationResult::Valid;
}
#endif

void FItemComponentData::OnItemStateChanged(const FInventoryItemEntryHandle& Handle, EUserFacingItemState NewState) const
{
	// If the state hasn't changed, don't do anything.
	if (CurrentState == NewState)
	{
		return;
	}

	FItemComponentData* MutableThis = const_cast<FItemComponentData*>(this);
	MutableThis->CurrentState = NewState;
}