// Author: Tom Werner (MajorT), 2025


#include "InventoryItemHandle.h"

void FInventoryItemHandle::GenerateNewUID()
{
	// Must be in C++ to avoid duplicate statics across execution units
	static uint32 GHandle = 1;
	UID = GHandle++;
}
