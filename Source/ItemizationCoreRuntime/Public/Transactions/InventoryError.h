// Author: Tom Werner (MajorT), 2025

#pragma once

#include "InventoryErrorCode.h"
#include "Internationalization/Internationalization.h"

#define MY_API ITEMIZATIONCORERUNTIME_API
#define LOCTEXT_NAMESPACE "InventoryError"

namespace UE::Itemization
{
using ErrorCodeType = uint64;
class FInventoryError;
	
class IInventoryErrorDetails
{
public:
	virtual ~IInventoryErrorDetails() = default;
	virtual FString GetLogString(const FInventoryError&) const = 0;
	virtual FText GetText(const FInventoryError&) const = 0;
};

class FInventoryErrorDetails : public IInventoryErrorDetails
{
public:
	FInventoryErrorDetails(FString&& InFriendlyErrorCode, FString&& InLogString, FText&& InText)
		: FriendlyErrorCode(InFriendlyErrorCode)
		, LogString(MoveTemp(InLogString))
		, Text(MoveTemp(InText))
	{
	}
	virtual ~FInventoryErrorDetails() override {};

public:
	virtual FString GetLogString(const FInventoryError&) const override
	{
		return LogString;
	}

	virtual FText GetText(const FInventoryError&) const override
	{
		return Text;
	}
	
protected:
	FString FriendlyErrorCode;
	FString LogString;
	FText Text;
};


class FInventoryError
{
public:
	explicit FInventoryError(bool) = delete;
	explicit FInventoryError(
		ErrorCodeType InErrorCode,
		const TSharedPtr<const IInventoryErrorDetails, ESPMode::ThreadSafe>& InDetails = nullptr,
		const TSharedPtr<const FInventoryError, ESPMode::ThreadSafe>& InInner = nullptr)
		: Details(InDetails)
		, Inner(InInner)
		, ErrorCode(InErrorCode)
	{
	}

	/** Returns the text stored in this error. */
	FText GetText() const
	{
		if (Details)
		{
			return Details->GetText(*this);
		}

		return FText::FromString(GetErrorId());
	}

	/** Returns the error system of this error. */
	ErrorCodeType GetSystem() const
	{
		return Errors::ErrorCodeSystem(ErrorCode);
	}

	/** Returns the error namespace of this error. */
	ErrorCodeType GetCategory() const
	{
		return Errors::ErrorCodeCategory(ErrorCode);
	}

	/** Returns the error code of this error. */
	ErrorCodeType GetValue() const
	{
		return Errors::ErrorCodeValue(ErrorCode);	
	}

	ErrorCodeType GetErrorCode() const
	{
		return ErrorCode;
	}

	/** Returns the entire error value as a string. */
	FString GetErrorId() const
	{
		FString ErrorId = FString::Printf(TEXT("%s"), *Errors::ErrorCode::ToString(ErrorCode));

		const FInventoryError* ThisInner = GetInner();
		while (ThisInner != nullptr)
		{
			ErrorId += FString::Printf(TEXT("-%s"), *Errors::ErrorCode::ToString(ThisInner->ErrorCode));
			ThisInner = Inner->GetInner();
		}

		return ErrorId;
	}

	/** Returns the log string stored in this error. */
	FString GetLogString(bool bIncludePrefix = true) const
	{
#if !NO_LOGGING
		FString MyLogString = TEXT("");

		if (Details)
		{
			FString LogPrefix = TEXT("");
			if (bIncludePrefix)
			{
				LogPrefix = FString::Printf(TEXT("[%s] "), *GetErrorId());
			}

			const FString LogString = Details->GetLogString(*this);
			MyLogString = FString::Printf(TEXT("%s%s"), *LogPrefix, *LogString);
		}
		else
		{
			MyLogString = FString::Printf(TEXT("[%s]"), *GetErrorId());
		}

		if (GetInner() != nullptr)
		{
			FString InnerLogString = GetInner()->GetLogString(false);
			if (!InnerLogString.IsEmpty())
			{
				return FString::Printf(TEXT("%s (%s)"), *MyLogString, *InnerLogString);
			}
		}

		return MyLogString;
#else
		return TEXT("");
#endif
	}

	/** Returns the error as a pointer. */
	const FInventoryError* GetInner() const
	{
		return Inner.IsValid() ? Inner.Get() : nullptr;
	}
	
private:
	TSharedPtr<const IInventoryErrorDetails, ESPMode::ThreadSafe> Details;
	TSharedPtr<const FInventoryError, ESPMode::ThreadSafe> Inner;
	ErrorCodeType ErrorCode;
};

MY_API bool operator==(const FInventoryError& Lhs, const FInventoryError& Rhs);
inline bool operator!=(const FInventoryError& Lhs, const FInventoryError& Rhs) { return !(Lhs == Rhs); }
MY_API bool operator==(const FInventoryError& InventoryError, ErrorCodeType OtherErrorCode);
inline bool operator==(ErrorCodeType OtherErrorCode, const FInventoryError& InventoryError) { return InventoryError == OtherErrorCode; }
inline bool operator!=(const FInventoryError& InventoryError, ErrorCodeType OtherErrorCode) { return !(InventoryError == OtherErrorCode); }
inline bool operator!=(ErrorCodeType OtherErrorCode, const FInventoryError& InventoryError) { return !(InventoryError == OtherErrorCode); }
	
inline FString ToLogString(const FInventoryError& Error)
{
	return Error.GetLogString();
}
}

#if UE_ENABLE_INCLUDE_ORDER_DEPRECATED_IN_5_2
#include "CoreMinimal.h"
#endif


#undef MY_API
#undef LOCTEXT_NAMESPACE