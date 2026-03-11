// Author: Tom Werner (dc: majort), 2026

#pragma once

#include "Logging/LogMacros.h"
#include <concepts>
#include <type_traits>
#include <utility>

class UObject;
class UWorld;

ITEMIZATIONCORE_API DECLARE_LOG_CATEGORY_EXTERN(LogItemization, Log, All);
ITEMIZATIONCORE_API DECLARE_LOG_CATEGORY_EXTERN(LogItemizationUI, Log, All);

namespace  UE::ItemizationCore
{
	ITEMIZATIONCORE_API FString GetNetContextString(const UObject* Obj);
}

#define NET_FMT "[%s]: "
#define NET_TEXT(Object, Format, ...) TEXT(NET_FMT Format), *UE::ItemizationCore::GetNetContextString(Object), ##__VA_ARGS__
#define ITEMIZATION_LOG(Verbosity, Format, ...) UE_LOG(LogItemization, Verbosity, TEXT(Format), ##__VA_ARGS__)
#define ITEMIZATION_UI_LOG(Verbosity, Format, ...) UE_LOG(LogItemizationUI, Verbosity, TEXT(Format), ##__VA_ARGS__)
