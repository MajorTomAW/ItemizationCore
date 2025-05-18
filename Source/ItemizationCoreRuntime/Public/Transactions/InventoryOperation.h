// Author: Tom Werner (MajorT), 2025

#pragma once

#include "CoreMinimal.h"
#include "InventoryOpHandle.h"
#include "InventoryResult.h"

class FInventoryError;

namespace Private
{
	template <typename Outer, typename OpType, typename LastResultType>
	class TInventoryOpBase
	{
	public:
		TInventoryOpBase(LastResultType& InLastResult)
			: LastResult(InLastResult)
		{
		}
		
	protected:
		LastResultType& LastResult;
	};

	template <typename Outer, typename OpType>
	class TInventoryOpBase<Outer, OpType, void>
	{
	public:
		TInventoryOpBase() {}
	};
}

class FInventoryOperation
{
public:
	virtual ~FInventoryOperation() {}
	virtual void SetError(FInventoryError&& Error) = 0;
};

template <typename OpType>
class TInventoryOperation
	: public Private::TInventoryOpBase<TInventoryOperation<OpType>, OpType, void>
	, public FInventoryOperation
	, public TSharedFromThis<TInventoryOperation<OpType>>
{
public:
	using ParamsType = typename OpType::Params;
	using ResultType = typename OpType::Result;

	TInventoryOperation(ParamsType&& Params)
		: SharedState(MakeShared<FInventoryOpSharedState>(MoveTemp(Params)))
	{
	}

	virtual ~TInventoryOperation() override {}

	bool IsComplete() const
	{
		return SharedState->State <= EInventoryOpState::Completed;
	}

	EInventoryOpState GetState() const
	{
		return SharedState->State;
	}

	const ParamsType& GetParams() const
	{
		return SharedState->Params;
	}

	TInventoryOperation<OpType>& GetOwningOp()
	{
		return *this;
	}

	TInventoryOpHandle<OpType> GetHandle()
	{
		return TInventoryOpHandle<OpType>(CreateSharedState());
	}

	virtual void SetError(FInventoryError&& Error) override
	{
		SetResultAndSate(TInventoryTransactionResult<OpType>(MoveTemp(Error)), EInventoryOpState::Completed);
	}

protected:
	void SetResultAndSate(TInventoryTransactionResult<OpType>&& Result, EInventoryOpState State)
	{
		SharedState->Result = MoveTemp(Result);
		SharedState->State = State;
	}
	
	class FInventoryOpSharedState
	{
	public:
		FInventoryOpSharedState(ParamsType&& InParams)
			: Params(MoveTemp(InParams))
		{
		}
		
		ParamsType Params;
		TInventoryTransactionResult<OpType> Result;
		EInventoryOpState State = EInventoryOpState::Invalid;

		bool IsComplete() const
		{
			return State >= EInventoryOpState::Completed;
		}
	};

	class FInventoryOpSharedHandleState
		: public Private::IInventoryOpSharedState<OpType>
		, public TSharedFromThis<FInventoryOpSharedHandleState>
	{
	public:
		FInventoryOpSharedHandleState(const TSharedRef<TInventoryOperation<OpType>>& InOp)
			: SharedState(InOp->SharedState)
			, Op(InOp)
		{
		}
		
		virtual EInventoryOpState GetState() const override
		{
			return SharedState->State;
		}
		
	private:
		TSharedRef<FInventoryOpSharedState> SharedState;
		TWeakPtr<TInventoryOperation<OpType>> Op;
	};

	TSharedRef<Private::IInventoryOpSharedState<OpType>> CreateSharedState()
	{
		TSharedRef<FInventoryOpSharedHandleState> NewHandleState = MakeShared<FInventoryOpSharedHandleState>(this->AsShared());
		SharedHandleStates.Add(NewHandleState);
		return StaticCastSharedRef<Private::IInventoryOpSharedState<OpType>>(NewHandleState);
	}

	TSharedRef<FInventoryOpSharedState> SharedState;
	TArray<TSharedRef<FInventoryOpSharedHandleState>> SharedHandleStates;
};


template <typename OpType>
using TInventoryOpRef = TSharedRef<TInventoryOperation<OpType>>;
template <typename OpType>
using TInventoryOpPtr = TSharedPtr<TInventoryOperation<OpType>>;