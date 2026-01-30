// Author: Tom Werner (dc: majort), 2026 January


#include "Inventory/SlottableInventory.h"


// Sets default values
ASlottableInventory::ASlottableInventory()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void ASlottableInventory::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ASlottableInventory::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

