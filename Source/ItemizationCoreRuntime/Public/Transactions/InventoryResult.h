// Author: Tom Werner (MajorT), 2025

#pragma once

#include "InventoryError.h"
#include "Misc/TVariant.h"

template <typename SuccessType, typename ErrorType>
class TInventoryResult
{
public:
	TInventoryResult() = default;
	explicit TInventoryResult(const SuccessType& OkValue)
		: Storage(TInPlaceType<SuccessType>(), OkValue)
	{
	}
	explicit TInventoryResult(SuccessType&& OkValue)
		: Storage(TInPlaceType<SuccessType>(), MoveTemp(OkValue))
	{
	}
	explicit TInventoryResult(const ErrorType& ErrorValue)
		: Storage(TInPlaceType<ErrorType>(), ErrorValue)
	{
	}
	explicit TInventoryResult(ErrorType&& ErrorValue)
		: Storage(TInPlaceType<ErrorType>(), MoveTemp(ErrorValue))
	{
	}

PRAGMA_DISABLE_DEPRECATION_WARNINGS
	TInventoryResult(const TInventoryResult& Other) = default;
	TInventoryResult(TInventoryResult&& Other) = default;

	TInventoryResult& operator=(const TInventoryResult& Other)
	{
		if (&Other != this)
		{
			Storage = Other.Storage;
		}
		return *this;
	}

	TInventoryResult& operator=(TInventoryResult&& Other)
	{
		if (&Other != this)
		{
			Storage = MoveTemp(Other.Storage);
		}
		return *this;
	}
PRAGMA_ENABLE_DEPRECATION_WARNINGS

	virtual ~TInventoryResult() = default;

public:
	/** Check if the value held in the result is a SuccessType. */
	bool IsOk() const
	{
		return Storage.template IsType<SuccessType>();
	}

	/** Check if the value held in the result is an ErrorType. */
	bool IsError() const
	{
		return Storage.template IsType<ErrorType>();
	}

	/** Returns the Ok value stored in the result. This mustn't be called on a result holding the error type */
	const SuccessType& GetOkValue() const
	{
		checkf(IsOk(), TEXT("Is is an error to call GetOkValue() on a TInventoryTransactionResult that does not hold an ok value. Please either check IsOk() or use TryGetOkValue"));
		return Storage.template Get<SuccessType>();
	}
	/** Returns the Ok value stored in the result. This mustn't be called on a result holding the error type */
	SuccessType& GetOkValue()
	{
		checkf(IsOk(), TEXT("Is is an error to call GetOkValue() on a TInventoryTransactionResult that does not hold an ok value. Please either check IsOk() or use TryGetOkValue"));
		return Storage.template Get<SuccessType>();
	}

	/** Returns the Error value stored in the result. This mustn't be called on a result holding the success type */
	const ErrorType& GetErrorValue() const
	{
		checkf(IsError(), TEXT("Is is an error to call GetErrorValue() on a TInventoryTransactionResult that does not hold an error value. Please either check IsError() or use TryGetErrorValue"));
		return Storage.template Get<ErrorType>();
	}
	/** Returns the Error value stored in the result. This mustn't be called on a result holding the success type */
	ErrorType& GetErrorValue()
	{
		checkf(IsError(), TEXT("Is is an error to call GetErrorValue() on a TInventoryTransactionResult that does not hold an error value. Please either check IsError() or use TryGetErrorValue"));
		return Storage.template Get<ErrorType>();
	}

	/** Tries to convert from TInventoryTransactionResult<Success, Error> to Success* if the result is successful. */
	const SuccessType* TryGetOkValue() const
	{
		return const_cast<TInventoryResult*>(this)->TryGetOkValue();
	}
	/** Tries to convert from TInventoryTransactionResult<Success, Error> to Success* if the result is successful. */
	SuccessType* TryGetOkValue()
	{
		return Storage.template TryGet<SuccessType>();
	}

	/** Tries to convert from TInventoryTransactionResult<Success, Error> to Error* if the result is erroneous. */
	const ErrorType* TryGetErrorValue() const
	{
		return const_cast<TInventoryResult*>(this)->TryGetErrorValue();
	}
	/** Tries to convert from TInventoryTransactionResult<Success, Error> to Error* if the result is erroneous. */
	ErrorType* TryGetErrorValue()
	{
		return Storage.template TryGet<ErrorType>();
	}

	/** Unwraps the result, returning the success value if one is held, otherwise returning the default value passed. */
	const SuccessType& GetOkOrDefault(const SuccessType& DefaultValue) const
	{
		return IsOk() ? GetOkValue() : DefaultValue;
	}
	
private:
	/** Location that the result's value is stored */
	TVariant<SuccessType, ErrorType> Storage;
};


template <typename OpType>
class TInventoryTransactionResult : public TInventoryResult<typename OpType::Result, FInventoryError>
{
public:
	using TInventoryResult<typename OpType::Result, FInventoryError>::TInventoryResult;
};

template <typename T>
FString ToLogString(const TInventoryTransactionResult<T>& Result)
{
	if (Result.IsOk())
	{
		return ToLogString(Result.GetOkValue());
	}

	if (Result.IsError())
	{
		return ToLogString(Result.GetErrorValue());
	}

	return TEXT("Unknown result type");
}