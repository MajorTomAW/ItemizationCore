// Copyright © 2025 MajorT. All Rights Reserved.

#pragma once

#include "EItemizationInventoryType.generated.h"

/** Enum describing the type of inventory system, its ownership, and replication behavior. */
UENUM(BlueprintType)
enum class EItemizationInventoryType : uint8
{
	/**
	 * The inventory is player-owned and will persist as long as the player exists.
	 * Player-owned inventories are typically replicated to the local owner and server only,
	 * unless it is explicitly required to replicate to other clients.
	 */
	Player,

	/**
	 * The inventory is world-based and will persist as long as the object owning it exists.
	 * As these types of inventories aren't player-owned, they're typically replicated to every relevant client.
	 */
	World,
};
