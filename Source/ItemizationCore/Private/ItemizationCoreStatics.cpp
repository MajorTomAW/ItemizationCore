// Author: Tom Werner (dc: majort), 2026 January


#include "ItemizationCoreStatics.h"

FInventorySlotId UItemizationCoreStatics::MakeInventorySlotId(int64 Row, int64 Column)
{
	// We had to expose this as int64 to get the full range of uint32
	return FInventorySlotId(static_cast<uint32>(Row), static_cast<uint32>(Column));
}
