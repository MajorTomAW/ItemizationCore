// Author: Tom Werner (MajorT), 2025

#pragma once

#include "Internationalization/Internationalization.h"

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
	FInventoryErrorDetails() = default;
	FInventoryErrorDetails(FString&& InLogString, FText&& InText)
		: LogString(MoveTemp(InLogString))
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
	FString LogString;
	FText Text;
};

class FInventoryError
{
public:
	explicit FInventoryError(bool) = delete;
	explicit FInventoryError(
		const TSharedPtr<const IInventoryErrorDetails, ESPMode::ThreadSafe>& InDetails = nullptr,
		const TSharedPtr<const FInventoryError, ESPMode::ThreadSafe>& InInner = nullptr)
		: Details(InDetails)
		, Inner(InInner)
	{
	}

	/** Returns the text stored in this error. */
	FText GetText() const
	{
		if (Details)
		{
			return Details->GetText(*this);
		}

		return FText::GetEmpty();
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
				//LogPrefix = FString::Printf(TEXT("[%s] "), *GetErrorId());
			}

			const FString LogString = Details->GetLogString(*this);
			MyLogString = FString::Printf(TEXT("%s%s"), *LogPrefix, *LogString);
		}
		else
		{
			//MyLogString = FString::Printf(TEXT("[%s]"), *GetErrorId());
		}

		if (Get() != nullptr)
		{
			FString InnerLogString = Get()->GetLogString(false);
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
	const FInventoryError* Get() const
	{
		return Inner.IsValid() ? Inner.Get() : nullptr;
	}
	
private:
	TSharedPtr<const IInventoryErrorDetails, ESPMode::ThreadSafe> Details;
	TSharedPtr<const FInventoryError, ESPMode::ThreadSafe> Inner;
};

inline FString ToLogString(const FInventoryError& Error)
{
	return Error.GetLogString();
}

#if UE_ENABLE_INCLUDE_ORDER_DEPRECATED_IN_5_2
#include "CoreMinimal.h"
#endif
