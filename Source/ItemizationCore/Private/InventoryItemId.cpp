// Author: Tom Werner (dc: majort), 2026


#include "InventoryItemId.h"

FInventoryItemId FInventoryItemId::InvalidId;

void FInventoryItemId::GenerateNewId()
{
	// Must be in C++ to avoid duplicate statics across execution units
	static uint32 GHandle = 1;
	UID = GHandle++;
}
