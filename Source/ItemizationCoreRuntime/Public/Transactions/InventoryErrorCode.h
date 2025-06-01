// Author: Tom Werner (MajorT), 2025

#pragma once

#include "CoreTypes.h"

class FString;

#define LOCTEXT_NAMESPACE "InventoryErrorDefinitions"
#define MY_API ITEMIZATIONCORERUNTIME_API

namespace UE::Itemization::Errors
{
	using ErrorCodeType = uint64;


	// error code as struct with bitfields
	struct FInventoryErrorCode
	{
		uint64 System : 4; // Engine, Game, Third Party Plugin
		uint64 Category : 28; // HRESULT, Http, nn::Result, etc
		uint64 Code : 32; // Error code value
	};

	MY_API uint64 ErrorCodeSystem(ErrorCodeType ErrorCode);
	MY_API uint64 ErrorCodeCategory(ErrorCodeType ErrorCode);
	MY_API uint64 ErrorCodeValue(ErrorCodeType ErrorCode);

	// define an error system
#define UE_ITEMIZATION_ERROR_SYSTEM(Name, InSystem, Value, Desc) \
namespace ErrorCode { namespace SystemCategory { static constexpr int Name = Value; } } \
namespace ErrorCode { namespace SystemCategory { static constexpr int Name##_System = UE::Itemization::Errors::ErrorCode::SystemCategory::InSystem; } }

	namespace ErrorCode
	{
		constexpr ErrorCodeType Create(uint32 System, uint32 Category, uint32 Code)
		{
			return ((System & 0xfull) << 60ull) | ((Category & 0x0fffffffull) << 32ull) | static_cast<ErrorCodeType>(
				Code);
		}

		namespace SystemCategory
		{
			constexpr ErrorCodeType Engine = 1;
			constexpr ErrorCodeType Game = 2;
			constexpr ErrorCodeType ThirdPartyPlugin = 3;
		}

		MY_API FString ToString(ErrorCodeType ErrorCode);
	}

	// define an error code
#define UE_ITEMIZATION_ERROR_Impl(System, Name, ErrorCodeValue, ErrorMsg, ErrorText) \
inline UE::Itemization::FInventoryError Name(const TOptional<UE::Itemization::FInventoryError>& Inner = TOptional<UE::Itemization::FInventoryError>()) \
{ \
TSharedPtr<UE::Itemization::FInventoryError, ESPMode::ThreadSafe> InnerPtr; \
if (Inner) \
{ \
InnerPtr = MakeShared<UE::Itemization::FInventoryError, ESPMode::ThreadSafe>(Inner.GetValue()); \
} \
return UE::Itemization::FInventoryError( \
UE::Itemization::Errors::ErrorCode::Create(ErrorCode::SystemCategory::System##_System, ErrorCode::SystemCategory::System, ErrorCodeValue), \
MakeShared<UE::Itemization::FInventoryErrorDetails, ESPMode::ThreadSafe>(FString(#System"."#Name), ErrorMsg, ErrorText));\
}


#define UE_ITEMIZATION_ERROR(System, Name, ErrorCodeValue, ErrorMsg, ErrorText) \
namespace ErrorCode { namespace System { static constexpr ErrorCodeType Name = Create(SystemCategory::System##_System, SystemCategory::System, ErrorCodeValue); } } \
UE_ITEMIZATION_ERROR_Impl(System, Name, ErrorCodeValue, ErrorMsg, ErrorText)
}

#undef MY_API
#undef LOCTEXT_NAMESPACE
