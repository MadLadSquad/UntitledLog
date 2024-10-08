#pragma once
#include <iostream>
#include <fstream>
#include <chrono>
#include <vector>
#include <functional>
#include <sstream>
#include <exception>

#ifdef MLS_EXPORT_LIBRARY
    #ifdef _WIN32
        #ifdef MLS_LIB_COMPILE
            #define MLS_PUBLIC_API __declspec(dllexport)
        #else
            #define MLS_PUBLIC_API __declspec(dllimport)
        #endif
    #else
        #define MLS_PUBLIC_API
    #endif
#else
    #define MLS_PUBLIC_API
#endif

namespace UVKLog
{
    enum [[maybe_unused]] LogColour
    {
        UVK_LOG_COLOUR_GREEN = 0,
        UVK_LOG_COLOUR_YELLOW = 1,
        UVK_LOG_COLOUR_RED = 2,
        UVK_LOG_COLOUR_WHITE = 3,
        UVK_LOG_COLOUR_BLUE = 4,
        UVK_LOG_COLOUR_NULL = 5
    };

    enum LogType
    {
        UVK_LOG_TYPE_WARNING = 1,
        UVK_LOG_TYPE_ERROR = 2,
        UVK_LOG_TYPE_NOTE = 4,
        UVK_LOG_TYPE_SUCCESS = 0,
        UVK_LOG_TYPE_MESSAGE = 3
    };

    enum [[maybe_unused]] LogOperations
    {
        // This is the default operation
        UVK_LOG_OPERATION_TERMINAL,
        UVK_LOG_OPERATION_FILE,
        UVK_LOG_OPERATION_FILE_AND_TERMINAL,
    };

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
    private:
        friend class Logger;
        friend class ImGuiConsole;

        template<bool bFile, typename... args>
        void agnostic(const char* message, LogType type, args&&... argv) noexcept
        {
            bool bError = false;
            if (type == UVK_LOG_TYPE_ERROR && bUsingErrors)
                bError = true;

            std::string output = "[" + getCurrentTime() + "] " + logColours[type + logTypeOffset] + ": " + message;
            std::stringstream ss;
            (ss << ... << argv);
            output += ss.str();

            if constexpr (bFile)
                fileout << output << std::endl;
            else
                std::cout << logColours[type] << output << logColours[logTypeOffset - 1] << std::endl;

            messageLog.emplace_back(output, type);

            if (bError)
            {
#ifdef NO_INSTANT_CRASH
                std::cin.get();
#endif
                std::terminate();
            }
        }

        std::ofstream fileout;
        bool bUsingErrors = false;
        std::vector<std::pair<std::string, LogType>> messageLog;

        std::vector<CommandType> commands;

        LogOperations operationType = UVK_LOG_OPERATION_TERMINAL;

        static std::string getCurrentTime() noexcept;
        void shutdownFileStream() noexcept;
    };

    inline LoggerInternal loggerInternal;

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
            if (loggerInternal.operationType == UVK_LOG_OPERATION_FILE_AND_TERMINAL)
            {
                loggerInternal.agnostic<false>(message, type, argv...);
                loggerInternal.agnostic<true>(message, type, argv...);
            }
            else if (loggerInternal.operationType == UVK_LOG_OPERATION_TERMINAL)
                loggerInternal.agnostic<false>(message, type, argv...);
            else
                loggerInternal.agnostic<true>(message, type, argv...);
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
        double duration = 0;

        std::chrono::time_point<std::chrono::high_resolution_clock> startPos;
    };
}
