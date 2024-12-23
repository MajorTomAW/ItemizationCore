// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/ItemComponentData.h"

#if WITH_EDITOR
EDataValidationResult FItemComponentData::IsDataValid(class FDataValidationContext& Context) const
{
	return EDataValidationResult::Valid;
}
#endif