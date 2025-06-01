// Author: Tom Werner (MajorT), 2025

#pragma once

#include "InventoryError.h"
#include "InventoryErrorCode.h"

#define LOCTEXT_NAMESPACE "InventoryErrors"

namespace UE::Itemization::Errors
{
	// Bunch of error definitions
	UE_ITEMIZATION_ERROR_SYSTEM(Common, Engine, 0x1, "Itemization Core")

	UE_ITEMIZATION_ERROR(Common, Success, 0, TEXT("success"), LOCTEXT("Success", "Success"))
	UE_ITEMIZATION_ERROR(Common, InvalidItemId, 0, TEXT("invalid_item_id"), LOCTEXT("InvalidItemId", "Bad Item ID"))
	UE_ITEMIZATION_ERROR(Common, InvalidParams, 9, TEXT("invalid_params"), LOCTEXT("InvalidParams", "Invalid Params"))
	UE_ITEMIZATION_ERROR(Common, NotImplemented, 14, TEXT("not_implemented"), LOCTEXT("NotImplemented", "Not implemented"))
	UE_ITEMIZATION_ERROR(Common, Cancelled, 16, TEXT("cancelled"), LOCTEXT("Cancelled", "Operation was cancelled"))
	UE_ITEMIZATION_ERROR(Common, Unknown, 22, TEXT("unknown"), LOCTEXT("Unknown", "Unknown Error"))
}

#undef LOCTEXT_NAMESPACE