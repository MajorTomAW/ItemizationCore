// Author: Tom Werner (MajorT), 2025

#pragma once

#include "CoreMinimal.h"

namespace UE::Itemization
{
	class FInventoryError;
	
	enum class EInventoryOpState : uint8
	{
		Invalid,
		Queued,
		Running,
		Completed,
		Cancelled,
	};

	/** Singular operation running on an inventory. */
	class IInventoryOp
	{
	public:
		virtual ~IInventoryOp() = default;

		virtual void Cancel() = 0;
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
			virtual void Cancel(const FInventoryError& Reason) = 0;
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

		void Cancel(const FInventoryError& Reason)
		{
			State->Cancel(Reason);
		}
	
	private:
		TSharedPtr<Private::IInventoryOpSharedState<OpType>> State;
	};
}