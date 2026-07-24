#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
    #define _CRT_SECURE_NO_WARNINGS
#endif

#include "ULog.hpp"
#include <ctime>
#include "ULogImGui.hpp"

#define TIME_COUNT(x) std::chrono::time_point_cast<std::chrono::microseconds>(x).time_since_epoch().count()

void ULog::Timer::start() noexcept
{
    // Store the start timestamp as a whole microsecond count. A double represents integers exactly up to 2^53, which
    // covers ~285 years of microseconds, so no precision is lost - and this avoids type-punning a chrono::time_point
    // through the double field that the C ABI (ULog_Timer) forces us to use.
    const auto tmp = std::chrono::high_resolution_clock::now();
    timer.startPos = static_cast<double>(TIME_COUNT(tmp));
}

void ULog::Timer::stop() noexcept
{
    const auto endTime = std::chrono::high_resolution_clock::now();
    timer.duration = (static_cast<double>(TIME_COUNT(endTime)) - timer.startPos) * 0.001;
}

ULog::Timer::~Timer() noexcept
{
    // Intentionally does not call stop(): the duration is unobservable once the Timer is destroyed, and calling stop()
    // here would clobber a value the user may have deliberately frozen with an explicit stop().
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
    auto& logger = LoggerInternal::get();
    logger.shutdownFileStream();

    // Constructing an std::ofstream from a null path is undefined behaviour (it reaches fopen(nullptr)), so bail out
    // before touching the stream. This is reachable from the C API, which forwards the pointer verbatim.
    if (file == nullptr)
    {
        Logger::log("failed to open log file '", ULOG_LOG_TYPE_WARNING, "(null)",
                    "'; file logging will be discarded until a valid file is set");
        return;
    }

    logger.fileout = std::ofstream(file);
    // A failed open leaves the stream in a bad state where every subsequent write is silently discarded, so warn
    // instead of letting FILE-mode logging vanish without a trace.
    if (!logger.fileout.is_open())
        Logger::log("failed to open log file '", ULOG_LOG_TYPE_WARNING, file,
                    "'; file logging will be discarded until a valid file is set");
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

void ULog::LoggerInternal::pushMessage(std::string msg, const LogType type) noexcept
{
    // The ImGui console renders messageLog through an ImGuiListClipper, which assumes every entry is exactly one row
    // tall. A message with embedded newlines would violate that and corrupt scrolling, so split it into one entry per
    // physical line here. Only the first line keeps the "[time] Type:" prefix; continuation lines are stored bare.
    // Note: because entries are now lines, maxLogMessages bounds physical lines rather than log calls.
    const size_t firstNewline = msg.find('\n');
    if (firstNewline == std::string::npos)
    {
        // Common case: single-line message, move it straight in with no copy.
        messageLog.emplace_back(std::move(msg), type);
        trimMessageLog();
        return;
    }

    size_t start = 0;
    for (size_t nl = firstNewline; nl != std::string::npos; nl = msg.find('\n', start))
    {
        messageLog.emplace_back(msg.substr(start, nl - start), type);
        start = nl + 1;
    }
    // Trailing segment after the last newline (empty if the message ended with '\n').
    messageLog.emplace_back(msg.substr(start), type);
    trimMessageLog();
}

void ULog::LoggerInternal::trimMessageLog() noexcept
{
    // A limit of 0 means "unbounded"
    if (maxLogMessages == 0 || messageLog.size() <= maxLogMessages)
        return;
    messageLog.erase(messageLog.begin(), messageLog.begin() + static_cast<std::ptrdiff_t>(messageLog.size() - maxLogMessages));
}

void ULog::LoggerInternal::handleError(const LogType type) noexcept
{
    // Mirror agnostic()'s early-out: when logging is disabled we produce no output at all, so we must not terminate
    // either - otherwise a disabled logger would crash the app on an error with nothing written anywhere.
    if (!bLoggingEnabled || type != ULOG_LOG_TYPE_ERROR || !bUsingErrors)
        return;
#ifdef ULOG_NO_INSTANT_CRASH
    std::cin.get();
#endif
    std::terminate();
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
