#include "CULog.h"
#include "../ULog.hpp"
#include <cstdio>

void ULog_Logger_setCrashOnError(const bool bError)
{
    ULog::Logger::setCrashOnError(bError);
}


void ULog_Logger_setEnableLogging(bool bEnable)
{
    ULog::Logger::setEnableLogging(bEnable);
}

void ULog_Logger_setCurrentLogFile(const char* file)
{
    ULog::Logger::setCurrentLogFile(file);
}

void ULog_Logger_setLogOperations(const ULog_LogOperations op)
{
    ULog::Logger::setLogOperation(op);
}

void ULog_Logger_setMaxLogMessages(const size_t max)
{
    ULog::Logger::setMaxLogMessages(max);
}

void ULog_Logger_log(const ULog_LogType type, const char* fmt, ...)
{
    va_list list;
    va_start(list, fmt);
    ULog_Logger_logV(type, fmt, list);
    va_end(list);
}

#ifdef _WIN32
int vasprintf(char** strp, const char* fmt, va_list ap)
{
    // Determine the length of the formatted string
    int len = _vscprintf(fmt, ap);
    if (len == -1) {
        return -1;
    }

    // Allocate memory for the formatted string (+1 for null-terminator)
    *strp = static_cast<char*>(malloc(static_cast<size_t>(len) + 1));
    if (!*strp) {
        return -1;
    }

    // Format the string and store it in the allocated buffer
    int result = vsnprintf(*strp, static_cast<size_t>(len) + 1, fmt, ap);
    if (result == -1) {
        free(*strp);
        *strp = nullptr;
        return -1;
    }

    return result; // Return the number of characters written (excluding null-terminator)
}
#endif

static void printToFile(const std::string& output) noexcept
{
    ULog::LoggerInternal::get().fileout << output << std::endl;
}

static void printToConsole(const std::string& output, const ULog_LogType type) noexcept
{
    printf("%s%s%s\n", ULog::logColours[type], output.c_str(), ULog::logColours[ULog::logTypeOffset - 1]);
}

void ULog_Logger_logV(const ULog_LogType type, const char* fmt, va_list list)
{
    auto& logger = ULog::LoggerInternal::get();
    if (!logger.bLoggingEnabled)
        return;
    // Guard against an out-of-range type indexing logColours out of bounds
    const ULog_LogType safeType = ULog::sanitizeLogType(type);
    std::string output = "[" + ULog::LoggerInternal::getCurrentTime() + "] " + ULog::logColours[safeType + ULog::logTypeOffset] + ": ";

    // Format the variadic message exactly once and append it to the header. This keeps the full message inside
    // "output" (so it makes it into messageLog for the ImGui console) and avoids consuming "list" more than once,
    // which would be undefined behaviour.
    char* buffer = nullptr;
    if (vasprintf(&buffer, fmt, list) != -1 && buffer != nullptr)
    {
        output += buffer;
        free(buffer);
    }

    if (logger.operationType == ULOG_LOG_OPERATION_FILE_AND_TERMINAL)
    {
        printToFile(output);
        printToConsole(output, safeType);
    }
    else if (logger.operationType == ULOG_LOG_OPERATION_FILE)
        printToFile(output);
    else
        printToConsole(output, safeType);

    logger.pushMessage(output, safeType);
    if (safeType == ULOG_LOG_TYPE_ERROR && logger.bUsingErrors)
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
