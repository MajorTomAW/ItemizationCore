// Author: Tom Werner (MajorT), 2025

#pragma once
#include "InventoryOp.h"
#include "InventoryTrackableOp.h"

namespace UE::Itemization
{
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
		TSharedRef<TInventoryOp<OpType>> MakeSharedOp(typename  OpType::FParams&& Params)
		{
			static int32 NextOpId = 0;

			TSharedRef<TInventoryOp<OpType>> NewOp =
				MakeShared<TInventoryOp<OpType>>(Forward<typename OpType::FParams>(Params));

			NewOp->OpIndex = ++NextOpId;
			
			Operations.Add(NewOp->OpIndex, NewOp);

			return NewOp;
		}

		/** Removes an operation from the cache. */
		void RemoveOp(int32 OpIndex)
		{
			Operations.Remove(OpIndex);
		}

		/** Removes an operation from the cache on the next tick. */
		void RemoveOpDeferred(int32 OpIndex)
		{
			// This is a no-op for now, but can be implemented to delay removal
			// until the next tick or some other condition.
			RemoveOp(OpIndex);
		}

		uint32 GetNumOps() const
		{
			return Operations.Num();
		}

		TArray<TSharedPtr<FInventoryOp>> GetPendingOperations() const
		{
			TArray<TSharedPtr<FInventoryOp>> Out;
			Operations.GenerateValueArray(Out);

			return Out;
		}
		
	private:
		TMap<int32, TSharedPtr<FInventoryOp>> Operations;
	};
}
