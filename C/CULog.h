#pragma once
#include "../ULogCommon.h"
#ifdef __cplusplus
extern "C"
{
#endif

    MLS_PUBLIC_API void ULog_Logger_setCrashOnError(bool bError);
    MLS_PUBLIC_API void ULog_Logger_setEnableLogging(bool bEnable);
    MLS_PUBLIC_API void ULog_Logger_setCurrentLogFile(const char* file);
    MLS_PUBLIC_API void ULog_Logger_setLogOperations(ULog_LogOperations op);
    MLS_PUBLIC_API void ULog_Logger_log(ULog_LogType type, const char* fmt, ...);
    MLS_PUBLIC_API void ULog_Logger_logV(ULog_LogType type, const char* fmt, va_list list);

    MLS_PUBLIC_API void ULog_Timer_start(ULog_Timer* timer);
    MLS_PUBLIC_API void ULog_Timer_stop(ULog_Timer* timer);
    MLS_PUBLIC_API double ULog_Timer_get(ULog_Timer* timer);

#ifdef __cplusplus
}
#endif