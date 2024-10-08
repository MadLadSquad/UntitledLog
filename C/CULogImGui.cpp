#include "CULogImGui.h"
#ifdef ULOG_IMGUI
#if __has_include(<imgui.h>)
#include "../ULogImGui.hpp"

ULog_CImGuiConsole ULog_ImGuiConsole_init()
{
    ULog::ImGuiConsole console{};
    return *reinterpret_cast<ULog_CImGuiConsole*>(&console);
}

#define CONCAST(x) reinterpret_cast<ULog::ImGuiConsole*>(x)

void ULog_ImGuiConsole_displayFull(ULog_CImGuiConsole* self, bool* bOpen, bool* bInteractingWithTextbox)
{
    CONCAST(self)->displayFull(*bOpen, bInteractingWithTextbox);
}

void ULog_ImGuiConsole_display(ULog_CImGuiConsole* self, bool* bInteractingWithTextbox)
{
    CONCAST(self)->display(bInteractingWithTextbox);
}

void ULog_addMessageToLow(const char* message, const ULog_LogType type)
{
    ULog::ImGuiConsole::addToMessageLog(message, type);
}

void ULog_addCommand(const char* cmd, const char* cmdHint, ULog_ImGuiConsole_CommandFunc func)
{
    ULog::LoggerInternal::get().commands.emplace_back(cmd, cmdHint, [func](const std::string& s) -> void
    {
        func(s.c_str());
    });
}

void ULog_setLogColour(ULog_CImGuiConsole* self, ULog_Vec4 colour, ULog_LogType type)
{
    CONCAST(self)->setLogColour(*reinterpret_cast<ImVec4*>(&colour), type);
}

#endif
#endif