// Author: Tom Werner (MajorT), 2025

#pragma once

#include "CoreMinimal.h"

enum class EInventoryOpState : uint8
{
	Invalid,
	Running,
	Completed
};

/** Singular operation running on an inventory. */
class IInventoryOp
{
public:
	virtual ~IInventoryOp() = default;
	virtual EInventoryOpState GetState() const = 0;
};

namespace Private
{
	/** Shared state class to be used to access and track operations. */
	template <typename OpType>
	class IInventoryOpSharedState
	{
	public:
		virtual ~IInventoryOpSharedState() {}
		virtual EInventoryOpState GetState() const = 0;
	};
}

/** Op handle for outside access to the state of an operation. */
template <typename OpType>
class TInventoryOpHandle
{
public:
	TInventoryOpHandle(TSharedRef<Private::IInventoryOpSharedState<OpType>>&& InSharedState)
		: State(MoveTemp(InSharedState))
	{
	}

	/** Returns the state of the op. */
	EInventoryOpState GetState() const
	{
		return State->GetState();
	}
private:
	TSharedPtr<Private::IInventoryOpSharedState<OpType>> State;
};