#pragma once
#ifdef ULOG_IMGUI
#if __has_include(<imgui.h>)
#include "../ULogCommon.h"

#ifdef __cplusplus
extern "C"
{
#endif

    typedef struct MLS_PUBLIC_API ULog_Vec4
    {
        float x;
        float y;
        float z;
        float w;
    } ULog_Vec4;

    typedef struct MLS_PUBLIC_API ULog_CImGuiConsole
    {
        ULog_Vec4 success;
        ULog_Vec4 warning;
        ULog_Vec4 error;
        ULog_Vec4 note;
        ULog_Vec4 message;
    } ULog_CImGuiConsole;

    typedef void(*ULog_ImGuiConsole_CommandFunc)(const char*);

    // UntitledImGuiFramework Event Safety - Any time
    MLS_PUBLIC_API ULog_CImGuiConsole ULog_ImGuiConsole_init();

    // UntitledImGuiFramework Event Safety - Any time
    MLS_PUBLIC_API void ULog_ImGuiConsole_displayFull(ULog_CImGuiConsole* self, bool* bOpen, bool* bInteractingWithTextbox);
    // UntitledImGuiFramework Event Safety - Any time
    MLS_PUBLIC_API void ULog_ImGuiConsole_display(ULog_CImGuiConsole* self, bool* bInteractingWithTextbox);

    // UntitledImGuiFramework Event Safety - Any time
    MLS_PUBLIC_API void ULog_addMessageToLow(const char* message, ULog_LogType type);
    // UntitledImGuiFramework Event Safety - Any time
    MLS_PUBLIC_API void ULog_addCommand(const char* cmd, const char* cmdHint, ULog_ImGuiConsole_CommandFunc func);

    // UntitledImGuiFramework Event Safety - Any time
    MLS_PUBLIC_API void ULog_setLogColour(ULog_CImGuiConsole* self, ULog_Vec4 colour, ULog_LogType type);

#ifdef __cplusplus
}
#endif
#endif
#endif