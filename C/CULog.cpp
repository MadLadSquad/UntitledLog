#include "CULog.h"
#include "../ULog.hpp"

void ULog_Logger_setCrashOnError(const bool bError)
{
    ULog::Logger::setCrashOnError(bError);
}

void ULog_Logger_setCurrentLogFile(const char* file)
{
    ULog::Logger::setCurrentLogFile(file);
}

void ULog_Logger_setLogOperations(const ULog_LogOperations op)
{
    ULog::Logger::setLogOperation(op);
}

void ULog_Logger_log(const ULog_LogType type, const char* fmt, ...)
{
    va_list list;
    va_start(list, fmt);
    ULog_Logger_logV(type, fmt, list);
    va_end(list);
}

void ULog_Logger_logV(const ULog_LogType type, const char* fmt, va_list va_list)
{
    auto& logger = ULog::LoggerInternal::get();
    const std::string output = "[" + ULog::LoggerInternal::getCurrentTime() + "] " + ULog::logColours[type + ULog::logTypeOffset] + ": ";

    if (logger.operationType == ULOG_LOG_OPERATION_FILE_AND_TERMINAL)
    {
        logger.fileout << output << std::endl;

        printf("%s%s", ULog::logColours[type], output.c_str());
        vprintf(fmt, va_list);
        printf("%s", ULog::logColours[ULog::logTypeOffset - 1]);
    }
    else if (logger.operationType == ULOG_LOG_OPERATION_FILE)
        logger.fileout << output << std::endl;
    else
    {
        printf("%s%s", ULog::logColours[type], output.c_str());
        vprintf(fmt, va_list);
        printf("%s", ULog::logColours[ULog::logTypeOffset - 1]);
    }

    logger.messageLog.emplace_back(output, type);
    if (type == ULOG_LOG_TYPE_ERROR && logger.bUsingErrors)
    {
#ifdef ULOG_NO_INSTANT_CRASH
        std::cin.get();
#endif
        std::terminate();
    }

}

void ULog_Timer_start(ULog_Timer* timer)
{
    reinterpret_cast<ULog::Timer*>(timer)->start();
}

void ULog_Timer_stop(ULog_Timer* timer)
{
    reinterpret_cast<ULog::Timer*>(timer)->stop();
}

double ULog_Timer_get(ULog_Timer* timer)
{
    return reinterpret_cast<ULog::Timer*>(timer)->get();
}
