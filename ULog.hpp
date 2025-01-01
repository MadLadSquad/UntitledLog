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

        template<bool bFile, typename... args>
        void agnostic(const char* message, LogType type, args&&... argv) noexcept
        {
            if (!bLoggingEnabled)
                return;
            std::string output = "[" + getCurrentTime() + "] " + logColours[type + logTypeOffset] + ": " + message;
            std::stringstream ss;
            (ss << ... << argv);
            output += ss.str();

            if constexpr (bFile)
                fileout << output << std::endl;
            else
                std::cout << logColours[type] << output << logColours[logTypeOffset - 1] << std::endl;

            messageLog.emplace_back(output, type);

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

        std::vector<CommandType> commands;

        LogOperations operationType = ULOG_LOG_OPERATION_TERMINAL;

        static std::string getCurrentTime() noexcept;
        void shutdownFileStream() noexcept;
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
                logger.agnostic<false>(message, type, argv...);
                logger.agnostic<true>(message, type, argv...);
            }
            else if (logger.operationType == ULOG_LOG_OPERATION_TERMINAL)
                logger.agnostic<false>(message, type, argv...);
            else
                logger.agnostic<true>(message, type, argv...);
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
