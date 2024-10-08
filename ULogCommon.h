#pragma once
#include <stdbool.h>
#include <stdlib.h>
#include <stddef.h>
#include <inttypes.h>
#include <stdarg.h>

#ifdef __cplusplus
extern "C"
{
#endif

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

    typedef enum [[maybe_unused]] ULog_LogColour
    {
        ULOG_LOG_COLOUR_GREEN = 0,
        ULOG_LOG_COLOUR_YELLOW = 1,
        ULOG_LOG_COLOUR_RED = 2,
        ULOG_LOG_COLOUR_WHITE = 3,
        ULOG_LOG_COLOUR_BLUE = 4,
        ULOG_LOG_COLOUR_NULL = 5
    } ULog_LogColour;

    typedef enum ULog_LogType
    {
        ULOG_LOG_TYPE_WARNING = 1,
        ULOG_LOG_TYPE_ERROR = 2,
        ULOG_LOG_TYPE_NOTE = 4,
        ULOG_LOG_TYPE_SUCCESS = 0,
        ULOG_LOG_TYPE_MESSAGE = 3
    } ULog_LogType;

    typedef enum [[maybe_unused]] ULog_LogOperations
    {
        // This is the default operation
        ULOG_LOG_OPERATION_TERMINAL,
        ULOG_LOG_OPERATION_FILE,
        ULOG_LOG_OPERATION_FILE_AND_TERMINAL,
    } ULog_LogOperations;

    typedef struct MLS_PUBLIC_API ULog_Timer
    {
        double duration;
        double startPos;
    } ULog_Timer;

#ifdef __cplusplus
}
#endif