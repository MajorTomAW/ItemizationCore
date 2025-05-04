// Author: Tom Werner (MajorT), 2025


#include "Transactions/InventoryItemTransactionBase.h"

FInventoryItemTransactionBase::FInventoryItemTransactionBase(AController* InInstigator)
	: Instigator(InInstigator)
{
}

bool FInventoryItemTransactionBase::Undo()
{
	return false;
}

bool FInventoryItemTransactionBase::Redo()
{
	return false;
}
