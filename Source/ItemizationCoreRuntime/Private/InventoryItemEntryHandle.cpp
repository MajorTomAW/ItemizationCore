// Fill out your copyright notice in the Description page of Project Settings.


#include "InventoryItemEntryHandle.h"

const FInventoryItemEntryHandle FInventoryItemEntryHandle::NullHandle;

FInventoryItemEntryHandle::FInventoryItemEntryHandle()
	: Handle(INDEX_NONE)
{
}

void FInventoryItemEntryHandle::GenerateNewHandle()
{
	// Must be in C++ to avoid duplicate statics across execution units
	static int32 GHandle = 1;
	Handle = GHandle++;
}
