// Copyright © 2025 MajorT. All Rights Reserved.


#include "UniqueItemHandle.h"

const FUniqueItemHandle FUniqueItemHandle::NullHandle = FUniqueItemHandle(); 

FUniqueItemHandle::FUniqueItemHandle()
	: Handle(INDEX_NONE)
{
}

void FUniqueItemHandle::GenerateNewHandle()
{
	// Must be in C++ to avoid duplicating statics across execution units.
	static int32 GHandle = 1;
	Handle = GHandle++;
}
