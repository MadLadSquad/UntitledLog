#include "CULog.h"
#include "../ULog.hpp"
#include <cstdio>

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

#ifdef _WIN32
// Copied from https://stackoverflow.com/questions/40159892/using-asprintf-on-windows
int vasprintf(char **strp, const char *fmt, va_list ap)
{
    // _vscprintf tells you how big the buffer needs to be
    int len = _vscprintf(fmt, ap);
    if (len == -1) {
        return -1;
    }
    size_t size = (size_t)len + 1;
    char* str = (char*)malloc(size);
    if (!str) {
        return -1;
    }
    // _vsprintf_s is the "secure" version of vsprintf
    int r = _vsprintf_s(str, len + 1, fmt, ap);
    if (r == -1) {
        free(str);
        return -1;
    }
    *strp = str;
    return r;
}
#endif

static void printToFile(const std::string& output, const char* fmt, va_list list) noexcept
{
    auto& logger = ULog::LoggerInternal::get();

    logger.fileout << output;
    char* buffer;
    vasprintf(&buffer, fmt, list);
    logger.fileout << buffer << std::endl;
    free(buffer);
}

static void printToConsole(const std::string& output, const ULog_LogType type, const char* fmt, va_list list) noexcept
{
    printf("%s%s", ULog::logColours[type], output.c_str());
    vprintf(fmt, list);
    printf("%s", ULog::logColours[ULog::logTypeOffset - 1]);
}

void ULog_Logger_logV(const ULog_LogType type, const char* fmt, va_list list)
{
    auto& logger = ULog::LoggerInternal::get();
    const std::string output = "[" + ULog::LoggerInternal::getCurrentTime() + "] " + ULog::logColours[type + ULog::logTypeOffset] + ": ";

    if (logger.operationType == ULOG_LOG_OPERATION_FILE_AND_TERMINAL)
    {
        printToFile(output, fmt, list);
        printToConsole(output, type, fmt, list);
    }
    else if (logger.operationType == ULOG_LOG_OPERATION_FILE)
        printToFile(output, fmt, list);
    else
        printToConsole(output, type, fmt, list);

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
