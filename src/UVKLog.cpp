#include "UVKLog.h"
#include <ctime>
#include "UVKLogImGui.h"

#if _MSC_VER && !__INTEL_COMPILER
    #define _CRT_SECURE_NO_WARNINGS
#endif

#define TIME_COUNT(x) std::chrono::time_point_cast<std::chrono::microseconds>(x).time_since_epoch().count()

void UVKLog::Timer::start() noexcept
{
    startPos = std::chrono::high_resolution_clock::now();
}

void UVKLog::Timer::stop() noexcept
{
    auto endTime = std::chrono::high_resolution_clock::now();
    duration = static_cast<double>(TIME_COUNT(endTime) - TIME_COUNT(startPos)) * 0.001;
}

UVKLog::Timer::~Timer() noexcept
{
    stop();
}

double UVKLog::Timer::get() const noexcept
{
    return duration;
}

void UVKLog::Logger::setCrashOnError(bool bError) noexcept
{
    loggerInternal.bUsingErrors = bError;
}

void UVKLog::Logger::setCurrentLogFile(const char* file) noexcept
{
    loggerInternal.shutdownFileStream();
    loggerInternal.fileout = std::ofstream(file);
}

void UVKLog::Logger::setLogOperation(LogOperations op) noexcept
{
    loggerInternal.operationType = op;
}

void UVKLog::Logger::log(const char* message, UVKLog::LogType type) noexcept
{
    log(message, type, "");
}

std::string UVKLog::LoggerInternal::getCurrentTime() noexcept
{
    auto now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());

    std::string realTime = std::ctime(&now);
    realTime.erase(24);

    return realTime;
}

void UVKLog::LoggerInternal::shutdownFileStream() noexcept
{
    fileout.close();
}

UVKLog::LoggerInternal::LoggerInternal() noexcept
{
#ifdef UVK_LOG_IMGUI
    const CommandType clear =
    {
        .cmd = "clear",
        .cmdHint = "Clears the scroll buffer",
        .func = [&](const std::string&){ messageLog.clear(); },
    };

    const CommandType help
    {
        .cmd = "help",
        .cmdHint = "Sends a help message",
        .func = UVKLog::ImGuiConsole::showHelpMessage
    };

    commands.emplace_back(clear);
    commands.emplace_back(help);
#endif
}

UVKLog::LoggerInternal::~LoggerInternal() noexcept
{
    shutdownFileStream();
}
