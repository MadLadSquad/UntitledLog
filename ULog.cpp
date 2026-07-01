#include "ULog.hpp"
#include <ctime>
#include "ULogImGui.hpp"

#if _MSC_VER && !__INTEL_COMPILER
    #define _CRT_SECURE_NO_WARNINGS
#endif

#define TIME_COUNT(x) std::chrono::time_point_cast<std::chrono::microseconds>(x).time_since_epoch().count()

void ULog::Timer::start() noexcept
{
    const auto tmp = std::chrono::high_resolution_clock::now();
    // scary pointer bullshit because of the C API
    timer.startPos = *static_cast<const double* const>(static_cast<const void* const>(&tmp));
}

void ULog::Timer::stop() noexcept
{
    const auto endTime = std::chrono::high_resolution_clock::now();
    timer.duration = static_cast<double>(TIME_COUNT(endTime) - TIME_COUNT(
        // scary pointer bullshit because of the C API
        *static_cast<const std::chrono::time_point<std::chrono::high_resolution_clock>* const>(
            static_cast<const void* const>(&timer.startPos)
        )
    )) * 0.001;
}

ULog::Timer::~Timer() noexcept
{
    stop();
}

double ULog::Timer::get() const noexcept
{
    return timer.duration;
}

void ULog::Logger::setCrashOnError(const bool bError) noexcept
{
    LoggerInternal::get().bUsingErrors = bError;
}

void ULog::Logger::setEnableLogging(const bool bEnable) noexcept
{
    LoggerInternal::get().bLoggingEnabled = bEnable;
}

void ULog::Logger::setCurrentLogFile(const char* file) noexcept
{
    LoggerInternal::get().shutdownFileStream();
    LoggerInternal::get().fileout = std::ofstream(file);
}

void ULog::Logger::setLogOperation(const LogOperations op) noexcept
{
    LoggerInternal::get().operationType = op;
}

void ULog::Logger::setMaxLogMessages(const size_t max) noexcept
{
    auto& logger = LoggerInternal::get();
    logger.maxLogMessages = max;
    // Apply the new limit immediately to any already-recorded messages
    logger.trimMessageLog();
}

void ULog::Logger::log(const char* message, const LogType type) noexcept
{
    log(message, type, "");
}

std::string ULog::LoggerInternal::getCurrentTime() noexcept
{
    const auto now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());

    // Use the thread-safe, reentrant conversion instead of std::ctime, which writes to a shared static buffer and
    // can return nullptr on failure (constructing a std::string from which is undefined behaviour).
    std::tm timeInfo{};
#ifdef _WIN32
    if (localtime_s(&timeInfo, &now) != 0)
        return {};
#else
    if (localtime_r(&now, &timeInfo) == nullptr)
        return {};
#endif

    // "%a %b %e %H:%M:%S %Y" reproduces std::ctime's "Www Mmm dd hh:mm:ss yyyy" layout without the trailing newline.
    char buffer[64] = {};
    if (std::strftime(buffer, sizeof(buffer), "%a %b %e %H:%M:%S %Y", &timeInfo) == 0)
        return {};

    return buffer;
}

void ULog::LoggerInternal::shutdownFileStream() noexcept
{
    fileout.close();
}

void ULog::LoggerInternal::pushMessage(const std::string& msg, const LogType type) noexcept
{
    messageLog.emplace_back(msg, type);
    trimMessageLog();
}

void ULog::LoggerInternal::trimMessageLog() noexcept
{
    // A limit of 0 means "unbounded"
    if (maxLogMessages == 0 || messageLog.size() <= maxLogMessages)
        return;
    messageLog.erase(messageLog.begin(), messageLog.begin() + static_cast<std::ptrdiff_t>(messageLog.size() - maxLogMessages));
}

ULog::LoggerInternal::LoggerInternal() noexcept
{
#ifdef ULOG_IMGUI
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
        .func = ULog::ImGuiConsole::showHelpMessage
    };

    commands.emplace_back(clear);
    commands.emplace_back(help);
#endif
}

ULog::LoggerInternal::~LoggerInternal() noexcept
{
    shutdownFileStream();
}

ULog::LoggerInternal* ULog::LoggerInternal::getWithCreate() noexcept
{
    static LoggerInternal logger{};
    return &logger;
}

ULog::LoggerInternal& ULog::LoggerInternal::get(LoggerInternal* lg) noexcept
{
    static LoggerInternal* logger = lg == nullptr ? getWithCreate() : lg;
    return *logger;
}
