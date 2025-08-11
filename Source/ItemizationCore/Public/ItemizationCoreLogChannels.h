// Author: Tom Werner (MajorT), 2025

#pragma once

#include "Logging/LogMacros.h"
#include <concepts>
#include <type_traits>
#include <utility>

class UObject;
class UWorld;

ITEMIZATIONCORE_API DECLARE_LOG_CATEGORY_EXTERN(LogItemization, Log, All);

namespace UE::ItemizationCore
{
	FString GetNetContextString(const UObject* Obj);
}

#define EXEC_INFO_FORMAT "%s: "
#define EXEC_INFO *FString(__FUNCTION__)

#define CONTEXT_EXEC_INFO_FORMAT "%s (%s): "
#define CONTEXT_EXEC_INFO *FString(__FUNCTION__), *GetNameSafe(this)

#define NET_EXEC_INFO_FORMAT "%s (%s)[%s]: "
#define NET_EXEC_INFO *FString(__FUNCTION__), *GetNameSafe(this), *UE::ItemizationCore::GetNetContextString(this)

////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// Logging Macros

#define _ITEMIZATION_LOG_IMPL(LogCategory, Verbosity, Format, ...) \
	UE_LOG(LogCategory, Verbosity, TEXT(EXEC_INFO_FORMAT Format), EXEC_INFO, ##__VA_ARGS__)

#define _ITEMIZATION_LOG_CONTEXT_IMPL(LogCategory, Verbosity, Format, ...) \
 	UE_LOG(LogCategory, Verbosity, TEXT(CONTEXT_EXEC_INFO_FORMAT Format), CONTEXT_EXEC_INFO, ##__VA_ARGS__)

#define _ITEMIZATION_LOG_NET_IMPL(LogCategory, Verbosity, Format, ...) \
	UE_LOG(LogCategory, Verbosity, TEXT(NET_EXEC_INFO_FORMAT Format), NET_EXEC_INFO, ##__VA_ARGS__)

/// LogItemization
#define ITEMIZATION_LOG(Format, ...) _ITEMIZATION_LOG_IMPL(LogItemization, Log, Format, ##__VA_ARGS__)
#define ITEMIZATION_LOG_CONTEXT(Format, ...) _ITEMIZATION_LOG_CONTEXT_IMPL(LogItemization, Log, Format, ##__VA_ARGS__)
#define ITEMIZATION_LOG_NET(Format, ...) _ITEMIZATION_LOG_NET_IMPL(LogItemization, Log, Format, ##__VA_ARGS__)
#define ITEMIZATION_VERBOSE(Format, ...) _ITEMIZATION_LOG_IMPL(LogItemization, Verbose, Format, ##__VA_ARGS__)
#define ITEMIZATION_VERBOSE_CONTEXT(Format, ...) _ITEMIZATION_LOG_CONTEXT_IMPL(LogItemization, Verbose, Format, ##__VA_ARGS__)
#define ITEMIZATION_VERBOSE_NET(Format, ...) _ITEMIZATION_LOG_NET_IMPL(LogItemization, Verbose, Format, ##__VA_ARGS__)
#define ITEMIZATION_WARN(Format, ...) _ITEMIZATION_LOG_IMPL(LogItemization, Warning, Format, ##__VA_ARGS__)
#define ITEMIZATION_WARN_CONTEXT(Format, ...) _ITEMIZATION_LOG_CONTEXT_IMPL(LogItemization, Warning, Format, ##__VA_ARGS__)
#define ITEMIZATION_WARN_NET(Format, ...) _ITEMIZATION_LOG_NET_IMPL(LogItemization, Warning, Format, ##__VA_ARGS__)
#define ITEMIZATION_ERROR(Format, ...) _ITEMIZATION_LOG_IMPL(LogItemization, Error, Format, ##__VA_ARGS__)
#define ITEMIZATION_ERROR_CONTEXT(Format, ...) _ITEMIZATION_LOG_CONTEXT_IMPL(LogItemization, Error, Format, ##__VA_ARGS__)
#define ITEMIZATION_ERROR_NET(Format, ...) _ITEMIZATION_LOG_NET_IMPL(LogItemization, Error, Format, ##__VA_ARGS__)
#define ITEMIZATION_FATAL(Format, ...) _ITEMIZATION_LOG_IMPL(LogItemization, Fatal, Format, ##__VA_ARGS__)
#define ITEMIZATION_FATAL_CONTEXT(Format, ...) _ITEMIZATION_LOG_CONTEXT_IMPL(LogItemization, Fatal, Format, ##__VA_ARGS__)
#define ITEMIZATION_FATAL_NET(Format, ...) _ITEMIZATION_LOG_NET_IMPL(LogItemization, Fatal, Format, ##__VA_ARGS__)
#define ITEMIZATION_DISPLAY(Format, ...) _ITEMIZATION_LOG_IMPL(LogItemization, Display, Format, ##__VA_ARGS__)
#define ITEMIZATION_DISPLAY_CONTEXT(Format, ...) _ITEMIZATION_LOG_CONTEXT_IMPL(LogItemization, Display, Format, ##__VA_ARGS__)
#define ITEMIZATION_DISPLAY_NET(Format, ...) _ITEMIZATION_LOG_NET_IMPL(LogItemization, Display, Format, ##__VA_ARGS__)