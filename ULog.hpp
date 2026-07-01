#pragma once
#include <iostream>
#include <fstream>
#include <chrono>
#include <vector>
#include <functional>
#include <sstream>
#include <exception>
#include "ULogCommon.h"

namespace ULog
{
    typedef ULog_LogColour LogColour;
    typedef ULog_LogOperations LogOperations;
    typedef ULog_LogType LogType;

    // The offset by which the names of the given message types are listed in the logColours array below
    constexpr uint8_t logTypeOffset = 6;

    // A constexpr list of strings, elements 0-5 are escape codes for colours, 6-11 are the messages used for the given
    // escape colours
    constexpr const char* logColours[] =
    {
        "\x1b[32m",
        "\x1b[33m",
        "\x1b[31m",
        "\x1b[37m",
        "\x1b[34m",
        "\x1b[0m",
        "Success",
        "Warning",
        "Error",
        "Message",
        "Note",
        "Null"
    };

    // Clamps a possibly out-of-range LogType to a valid value so it can never index logColours out of bounds.
    // Valid types are numerically [ULOG_LOG_TYPE_SUCCESS(0), ULOG_LOG_TYPE_NOTE(4)].
    constexpr LogType sanitizeLogType(LogType type) noexcept
    {
        return (static_cast<int>(type) < ULOG_LOG_TYPE_SUCCESS || static_cast<int>(type) > ULOG_LOG_TYPE_NOTE)
                   ? ULOG_LOG_TYPE_MESSAGE : type;
    }

    struct MLS_PUBLIC_API CommandType
    {
        std::string cmd; // the name of the command;
        std::string cmdHint; // shown in the help message
        std::function<void(const std::string&)> func; // executes the command instructions
    };

    class MLS_PUBLIC_API LoggerInternal
    {
    public:
        LoggerInternal() noexcept;
        ~LoggerInternal() noexcept;

        static LoggerInternal* getWithCreate() noexcept;
        static LoggerInternal& get(LoggerInternal* lg = nullptr) noexcept;

        template<bool bFile, bool bRecord, typename... args>
        void agnostic(const char* message, LogType type, args&&... argv) noexcept
        {
            if (!bLoggingEnabled)
                return;
            type = sanitizeLogType(type);
            std::string output = "[" + getCurrentTime() + "] " + logColours[type + logTypeOffset] + ": " + message;
            std::stringstream ss;
            (ss << ... << argv);
            output += ss.str();

            if constexpr (bFile)
                fileout << output << std::endl;
            else
                std::cout << logColours[type] << output << logColours[logTypeOffset - 1] << std::endl;

            // Only record to the message log once per logged line. In FILE_AND_TERMINAL mode agnostic() runs twice
            // (once per stream), so the caller records on exactly one of those calls to avoid duplicate entries.
            if constexpr (bRecord)
                pushMessage(output, type);

            if (type == ULOG_LOG_TYPE_ERROR && bUsingErrors)
            {
#ifdef ULOG_NO_INSTANT_CRASH
                std::cin.get();
#endif
                std::terminate();
            }
        }

        std::ofstream fileout;
        bool bUsingErrors = false;
        bool bLoggingEnabled = true;
        std::vector<std::pair<std::string, LogType>> messageLog;

        // Maximum number of entries kept in messageLog. When exceeded, the oldest entries are dropped. A value of 0
        // disables the limit (unbounded growth). Defaults to 1000.
        size_t maxLogMessages = 1000;

        std::vector<CommandType> commands;

        // Backing storage for the ImGui console's command input box. Lives on the singleton (not as a member of
        // ImGuiConsole) because ImGuiConsole must stay layout-compatible with the C struct ULog_CImGuiConsole.
        std::string consoleCommand;

        LogOperations operationType = ULOG_LOG_OPERATION_TERMINAL;

        static std::string getCurrentTime() noexcept;
        void shutdownFileStream() noexcept;

        // Appends an entry to messageLog and trims the oldest entries so the log never exceeds maxLogMessages
        void pushMessage(const std::string& msg, LogType type) noexcept;

        // Drops the oldest entries so messageLog holds at most maxLogMessages entries (no-op when the limit is 0)
        void trimMessageLog() noexcept;
    };

    /**
     * @brief Logs a message to the terminal, a file or both
     * @note UntitledImGuiFramework Event Safety - Any time
     */
    class MLS_PUBLIC_API Logger
    {
    public:
        // If set to true calling log with the UVK_LOG_TYPE_ERROR will terminate the application
        // UntitledImGuiFramework Event Safety - Any time
        static void setCrashOnError(bool bError) noexcept;

        // If set to false, calling any log function will not produce any output
        // UntitledImGuiFramework Event Safety - Any time
        static void setEnableLogging(bool bEnable) noexcept;

        // Sets the current file to which we should log to if logging to files is enabled
        // UntitledImGuiFramework Event Safety - Any time
        static void setCurrentLogFile(const char* file) noexcept;

        // Sets the current log operation, useful for enabling/disabling logging to different streams
        // UntitledImGuiFramework Event Safety - Any time
        static void setLogOperation(LogOperations op) noexcept;

        // Sets the maximum number of messages retained in the in-memory log (used by the ImGui console). When the log
        // grows past this limit, the oldest messages are dropped. Passing 0 disables the limit. Defaults to 1000.
        // UntitledImGuiFramework Event Safety - Any time
        static void setMaxLogMessages(size_t max) noexcept;

        /**
         * @brief Logs a message and a templated variadic list of arguments to a stream depending on the current log
         * operation
         * @tparam args - A templated variadic arguments list
         * @param message - The initial message to be printed
         * @param type - The log type
         * @param argv - The templated variadic list that will be unrolled into the given stream
         * @note  UntitledImGuiFramework Event Safety - Any time
         */
        template<typename... args>
        static void log(const char* message, LogType type, args&&... argv) noexcept
        {
            auto& logger = LoggerInternal::get();
            if (logger.operationType == ULOG_LOG_OPERATION_FILE_AND_TERMINAL)
            {
                // Record on the terminal call; the file call must not record again (would duplicate the entry)
                logger.agnostic<false, true>(message, type, argv...);
                logger.agnostic<true, false>(message, type, argv...);
            }
            else if (logger.operationType == ULOG_LOG_OPERATION_TERMINAL)
                logger.agnostic<false, true>(message, type, argv...);
            else
                logger.agnostic<true, true>(message, type, argv...);
        }

        // Specialization where we don't use the additional templated arguments, look at the log above for documentation
        static void log(const char* message, LogType type) noexcept;
    };

    /**
     * @brief A small Timer class to track how much time a task takes
     * @note UntitledImGuiFramework Event Safety - Any time
     */
    class MLS_PUBLIC_API Timer
    {
    public:
        // Starts recording time
        // UntitledImGuiFramework Event Safety - Any time
        void start() noexcept;

        // Stops recording time. Doesn't "stop" the recording but rather just saves the time it took. This allows you to
        // call this function multiple times and use the "get" function to get the duration, which allows you to do if
        // checks on how long something took. To reset the clock just call start again
        //
        // UntitledImGuiFramework Event Safety - Any time
        void stop() noexcept;

        // Returns the duration time between starting the timer and the last
        // UntitledImGuiFramework Event Safety - Any time
        [[nodiscard]] double get() const noexcept;
        // UntitledImGuiFramework Event Safety - Any time
        ~Timer() noexcept;
    private:
        ULog_Timer timer{};
    };
}
