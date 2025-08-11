// Author: Tom Werner (MajorT), 2025

#pragma once

namespace Private
{
		
}
	
class FInventoryOp
{
public:
	virtual ~FInventoryOp() {}

	int32 OpIndex = INDEX_NONE;
	FString OpName;
	
	//@TODO: This is just experimental for testing right now.
	uint8 bPendingRemoval : 1 = false;
	friend class FInventoryOpCache;
	double ConstructionTime = 0.f;
};

template <typename OpType>
class TInventoryOp
	: public FInventoryOp
	, public TSharedFromThis<TInventoryOp<OpType>>
{
public:
	using ParamsType = typename OpType::FParams;
	using ResultType = typename OpType::FResult;

	TInventoryOp(ParamsType&& InParams)
		: Params(MoveTemp(InParams))
	{
		OpName = FString(OpType::Name);
		ConstructionTime = FPlatformTime::Seconds();
	}

	ParamsType Params;
	ResultType Result;
};


template <typename OpType>
using TInventoryOpRef = TSharedRef<TInventoryOp<OpType>>;
template <typename OpType>
using TInventoryOpPtr = TSharedPtr<TInventoryOp<OpType>>;