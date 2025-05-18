// Author: Tom Werner (MajorT), 2025

#pragma once

#include "CoreMinimal.h"
#include "InventoryOperation.h"
#include "InventoryOpHandle.h"

class IInventoryDataOp
{
public:
	virtual ~IInventoryDataOp() {}

	template <typename T>
	const T* Get() const
	{
		return static_cast<const T*>(GetData());
	}

	template <typename T>
	const T& GetRef() const
	{
		const T* Value = Get<T>();
		check(Value != nullptr);
		return *Value;
	}

protected:
	virtual const void* GetData() const = 0;
};

template <typename T, typename BaseType = IInventoryDataOp>
class TInventoryDataOp : public BaseType
{
public:
	using ValueType = std::remove_reference_t<T>;

	TInventoryDataOp(const ValueType& InData)
		: Data(InData)
	{
	}
	TInventoryDataOp(ValueType&& InData)
		: Data(MoveTemp(InData))
	{
	}

	const T& GetDataRef() const
	{
		return Data;
	}

protected:
	virtual const void* GetData() const override
	{
		return &Data;
	}
	T Data;
};

/** Cache used to store multiple inventory operations. */
class FInventoryOpCache
{
public:
	/** Create a new operation. */
	template <typename OpType>
	TInventoryOpRef<OpType> GetOperation(typename OpType::Params&& Params)
	{
		TSharedRef<TInventoryOperation<OpType>> Operation =
			MakeShared<TInventoryOperation<OpType>>(MoveTemp(Params));

		return Operation;
	}
protected:
private:
	class IWrappedInventoryOp : public IInventoryDataOp
	{
	public:
		virtual bool IsExpired() const = 0;
	};

	template <typename OpType>
	class TWrappedInventoryOp : public TInventoryDataOp<TSharedRef<TInventoryOpHandle<OpType>>, IWrappedInventoryOp>
	{
	public:
		template <typename... ParamTypes>
		TWrappedInventoryOp(ParamTypes&&... Params)
			: TInventoryDataOp<TSharedRef<TInventoryOperation<OpType>>
			, IWrappedInventoryOp>(MakeShared<TInventoryOperation<OpType>>(Forward<ParamTypes>(Params)...))
		{
		}

		virtual bool IsExpired() override
		{
			return false;
		}
	};
	
	TMap<int32, TUniquePtr<IWrappedInventoryOp>> Operations;
};