// Author: Tom Werner (MajorT), 2025


#include "Transactions/InventoryError.h"
#include "Transactions/InventoryErrorCode.h"
#include "Transactions/InventoryErrorDefinitions.h"

namespace UE::Itemization
{
	namespace Errors
	{
		uint64 ErrorCodeSystem(ErrorCodeType ErrorCode) { return (ErrorCode >> 60) & 0x0full; }

		uint64 ErrorCodeCategory(ErrorCodeType ErrorCode) {return (ErrorCode >> 32) & 0x0fffffffull; }

		uint64 ErrorCodeValue(ErrorCodeType ErrorCode) { return ErrorCode & 0xffffffffull; }
		
		namespace ErrorCode
		{
			FString ToString(ErrorCodeType ErrorCode)
			{
				const uint64 System = ErrorCodeSystem(ErrorCode);
				const uint64 Category = ErrorCodeCategory(ErrorCode);
				const uint64 Value = ErrorCodeValue(ErrorCode);
				return FString::Printf(TEXT("%llx.%llx.%llx"), System, Category, Value);
			}
		}
	}

	bool operator==(const FInventoryError& Lhs, const FInventoryError& Rhs)
	{
		const FInventoryError * Error = &Lhs;
		while(Error)
		{
			if (Rhs == Error->GetErrorCode())
			{
				return true;
			}
			Error = Error->GetInner();
		}
		return false;
	}

	bool operator==(const FInventoryError& Lhs, ErrorCodeType OtherErrorCode)
	{
		if (Lhs.GetErrorCode() == OtherErrorCode)
		{
			return true;
		}

		const FInventoryError* LoopInner = Lhs.GetInner();
		while (LoopInner != nullptr)
		{
			if (LoopInner->GetErrorCode() == OtherErrorCode)
			{
				return true;
			}
			LoopInner = LoopInner->GetInner();
		}

		return false;
	}
}

