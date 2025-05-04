// Author: Tom Werner (MajorT), 2025

#pragma once

#include "Logging/LogMacros.h"
#include <concepts>
#include <type_traits>
#include <utility>

class UObject;
class UWorld;

ITEMIZATIONCORERUNTIME_API DECLARE_LOG_CATEGORY_EXTERN(LogItemization, Log, All);

namespace UE::ItemizationCore
{
	FString GetNetContextString(const UObject* Obj);
}

#define ITEMIZATION_LOG(Format, ...)				UE_LOG(LogItemization, Log, TEXT(Format), ##__VA_ARGS__)
#define ITEMIZATION_WARN(Format, ...)				UE_LOG(LogItemization, Warning, TEXT(Format), ##__VA_ARGS__)
#define ITEMIZATION_DISPLAY(Format, ...)			UE_LOG(LogItemization, Display, TEXT(Format), ##__VA_ARGS__)
#define ITEMIZATION_ERROR(Format, ...)			UE_LOG(LogItemization, Error, TEXT(Format), ##__VA_ARGS__)
#define ITEMIZATION_FATAL(Format, ...)			UE_LOG(LogItemization, Fatal, TEXT(Format), ##__VA_ARGS__)
#define ITEMIZATION_VERBOSE(Format, ...)			UE_LOG(LogItemization, Verbose, TEXT(Format), ##__VA_ARGS__)
#define ITEMIZATION_VVERBOSE(Format, ...)			UE_LOG(LogItemization, VeryVerbose, TEXT(Format), ##__VA_ARGS__)

#define EXEC_INFO_FORMAT "%s: "
#define EXEC_INFO *FString(__FUNCTION__)

#define _ITEMIZATION_S_LOG_IMPL(Verbosity, Format, ...) \
	UE_LOG(LogItemization, Verbosity, TEXT(EXEC_INFO_FORMAT Format), EXEC_INFO, ##__VA_ARGS__)
	
#define ITEMIZATION_S_LOG(Format, ...)				_ITEMIZATION_S_LOG_IMPL(Log, Format, ##__VA_ARGS__)
#define ITEMIZATION_S_WARN(Format, ...)				_ITEMIZATION_S_LOG_IMPL(Warning, Format, ##__VA_ARGS__)
#define ITEMIZATION_S_DISPLAY(Format, ...)			_ITEMIZATION_S_LOG_IMPL(Display, Format, ##__VA_ARGS__)
#define ITEMIZATION_S_ERROR(Format, ...)			_ITEMIZATION_S_LOG_IMPL(Error, Format, ##__VA_ARGS__)
#define ITEMIZATION_S_FATAL(Format, ...)			_ITEMIZATION_S_LOG_IMPL(Fatal, Format, ##__VA_ARGS__)
#define ITEMIZATION_S_VERBOSE(Format, ...)			_ITEMIZATION_S_LOG_IMPL(Verbose, Format, ##__VA_ARGS__)
#define ITEMIZATION_S_VVERBOSE(Format, ...)			_ITEMIZATION_S_LOG_IMPL(VeryVerbose, Format, ##__VA_ARGS__)

#define N_EXEC_INFO_FORMAT "[%s] %s: "
#define N_EXEC_INFO *UE::ItemizationCore::GetNetContextString(this), *FString(__FUNCTION__)

#define _ITEMIZATION_N_LOG_IMPL(Verbosity, Format, ...) \
	UE_LOG(LogItemization, Verbosity, TEXT(N_EXEC_INFO_FORMAT Format), N_EXEC_INFO, ##__VA_ARGS__)

#define ITEMIZATION_N_LOG(Format, ...)				_ITEMIZATION_N_LOG_IMPL(Log, Format, ##__VA_ARGS__)
#define ITEMIZATION_N_WARN(Format, ...)				_ITEMIZATION_N_LOG_IMPL(Warning, Format, ##__VA_ARGS__)
#define ITEMIZATION_N_DISPLAY(Format, ...)			_ITEMIZATION_N_LOG_IMPL(Display, Format, ##__VA_ARGS__)
#define ITEMIZATION_N_ERROR(Format, ...)			_ITEMIZATION_N_LOG_IMPL(Error, Format, ##__VA_ARGS__)
#define ITEMIZATION_N_FATAL(Format, ...)			_ITEMIZATION_N_LOG_IMPL(Fatal, Format, ##__VA_ARGS__)
#define ITEMIZATION_N_VERBOSE(Format, ...)			_ITEMIZATION_N_LOG_IMPL(Verbose, Format, ##__VA_ARGS__)
#define ITEMIZATION_N_VVERBOSE(Format, ...)			_ITEMIZATION_N_LOG_IMPL(VeryVerbose, Format, ##__VA_ARGS__)