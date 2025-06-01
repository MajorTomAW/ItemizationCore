// Author: Tom Werner (MajorT), 2025

#pragma once

#include "Misc/GeneratedTypeName.h"

namespace UE::Itemization
{
	template <typename DataType>
	struct TInventoryTypeInfo
	{
		static inline const TCHAR* GetTypeName()
		{
			return GetGeneratedTypeName(std::remove_reference_t<DataType>());
		}

		static inline bool IsA(const TCHAR* TypeName)
		{
			if (TypeName == GetTypeName())
			{
				return true;
			}

			return false;
		}
	};
	
	namespace Private
	{
		// Map class for type names and values. For data of any type
		class FInventoryOpData
		{
		public:
			struct FOpDataKey;
			
			template <typename DataType>
			void Set(const FString& Key, DataType&& InData)
			{
				Data.Add(FOpDataKey{TInventoryTypeInfo<DataType>::GetTypeName(), Key},
					MakeUnique<TData<DataType>>(MoveTemp(InData)));
			}

			template <typename DataType>
			void Set(const FString& Key, const DataType& InData)
			{
				Data.Add(FOpDataKey{TInventoryTypeInfo<DataType>::GetTypeName(), Key},
					MakeUnique<TData<DataType>>(InData));
			}

			template <typename DataType>
			const DataType* Get(const FString& Key)
			{
				if (auto Value = Data.Find(FOpDataKey{TInventoryTypeInfo<DataType>::GetTypeName(), Key}))
				{
					return static_cast<const DataType*>((*Value)->GetData());
				}
				
				return nullptr;
			}

			// Key used to identify the data in the map.
			struct FOpDataKey
			{
				const TCHAR* Name = nullptr;
				FString Key;

				bool operator==(const FOpDataKey& Rhs) const
				{
					return Key == Rhs.Key && Name == Rhs.Name;
				}
			};

		private:
			// Data interface that can store any type of data.
			class IData
			{
			public:
				virtual ~IData() {}
				virtual const TCHAR* GetTypeName() = 0;
				virtual void* GetData() = 0;

				template <typename DataType>
				const DataType* Get()
				{
					if (GetTypeName() == TInventoryTypeInfo<DataType>::GetTypeName())
					{
						return static_cast<const DataType*>(this);
					}
					
					return nullptr;
				}
			};

			template <typename DataType>
			class TData : public IData
			{
			public:
				TData(const DataType& InData)
					: Data(InData)
				{
				}

				TData(DataType&& InData)
					: Data(MoveTemp(InData))
				{
				}

				virtual const TCHAR* GetTypeName() override
				{
					return TInventoryTypeInfo<DataType>::GetTypeName();
				}

				virtual void* GetData() override
				{
					return &Data;
				}

			private:
				DataType Data;
			};

			friend uint32 GetTypeHash(const FOpDataKey& Key);
			TMap<FOpDataKey, TUniquePtr<IData>> Data;
		};

		inline uint32 GetTypeHash(const FInventoryOpData::FOpDataKey& Key)
		{
			using ::GetTypeHash;
			return HashCombine(GetTypeHash(*Key.Name), GetTypeHash(Key.Key));
		}
	}
}