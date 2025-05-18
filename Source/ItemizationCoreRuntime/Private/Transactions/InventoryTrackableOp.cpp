// Author: Tom Werner (MajorT), 2025


#include "Transactions/InventoryTrackableOp.h"

FInventoryTrackableOp::FInventoryTrackableOp(AController* InInstigator)
	: Instigator(InInstigator)
{
}

bool FInventoryTrackableOp::Undo()
{
	return false;
}

bool FInventoryTrackableOp::Redo()
{
	return false;
}
