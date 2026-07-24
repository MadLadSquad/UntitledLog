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
// static so this common polyfill name can't collide with an identically-named vasprintf shim from another library
// linked into the same binary (a duplicate-symbol error at best, silent one-definition-rule breakage at worst).
static int vasprintf(char** strp, const char* fmt, va_list ap)
{
    // Determine the length of the formatted string
    int len = _vscprintf(fmt, ap);
    if (len < 0) {
        return -1;
    }

    // Allocate memory for the formatted string (+1 for null-terminator)
    *strp = static_cast<char*>(malloc(static_cast<size_t>(len) + 1));
    if (!*strp) {
        return -1;
    }

    // Format the string and store it in the allocated buffer
    int result = vsnprintf(*strp, static_cast<size_t>(len) + 1, fmt, ap);
    if (result < 0) {
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
    // vasprintf() with a null format is undefined behaviour; substitute a placeholder so a null pointer forwarded from
    // a caller can't reach it.
    if (fmt == nullptr)
        fmt = "(null)";
    // Guard against an out-of-range type indexing logColours out of bounds
    const ULog_LogType safeType = ULog::sanitizeLogType(type);
    std::string output = "[" + ULog::LoggerInternal::getCurrentTime() + "] " + ULog::logColours[safeType + ULog::logTypeOffset] + ": ";

    // Format the variadic message exactly once and append it to the header. This keeps the full message inside
    // "output" (so it makes it into messageLog for the ImGui console) and avoids consuming "list" more than once,
    // which would be undefined behaviour.
    char* buffer = nullptr;
    // vasprintf only guarantees a negative return on failure, not exactly -1, so test the sign rather than a magic value.
    if (vasprintf(&buffer, fmt, list) >= 0 && buffer != nullptr)
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

    logger.pushMessage(std::move(output), safeType);
    // Defer crash-on-error to the shared handler so the C and C++ paths terminate under identical conditions.
    logger.handleError(safeType);
}

// The C API reinterpret_casts between ULog_Timer* and ULog::Timer*; that is only valid while the C++ wrapper stays a
// transparent hull around the C struct. Adding any member (or a vtable) to ULog::Timer breaks this - fail loudly here.
static_assert(sizeof(ULog::Timer) == sizeof(ULog_Timer),
              "ULog::Timer must stay layout-compatible with ULog_Timer for the C API casts to be valid");

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
