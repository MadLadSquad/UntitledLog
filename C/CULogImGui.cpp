#include "CULogImGui.h"
#ifdef ULOG_IMGUI
#if __has_include(<imgui.h>)
#include "../ULogImGui.hpp"

// Every function below reinterpret_casts between ULog_CImGuiConsole and ULog::ImGuiConsole. That only holds while the
// two stay layout-identical (five ULog_Vec4/ImVec4 colours), so pin the size here - adding a member or a vtable to
// ULog::ImGuiConsole would otherwise corrupt memory silently.
static_assert(sizeof(ULog::ImGuiConsole) == sizeof(ULog_CImGuiConsole),
              "ULog::ImGuiConsole must stay layout-compatible with ULog_CImGuiConsole for the C API casts to be valid");

ULog_CImGuiConsole ULog_ImGuiConsole_init()
{
    ULog::ImGuiConsole console{};
    return *reinterpret_cast<ULog_CImGuiConsole*>(&console);
}

#define CONCAST(x) reinterpret_cast<ULog::ImGuiConsole*>(x)

void ULog_ImGuiConsole_displayFull(ULog_CImGuiConsole* self, bool* bOpen, bool* bInteractingWithTextbox)
{
    // displayFull takes bOpen by reference, so a null pointer here would be an immediate dereference of nullptr
    if (bOpen == nullptr)
        return;
    CONCAST(self)->displayFull(*bOpen, bInteractingWithTextbox);
}

void ULog_ImGuiConsole_display(ULog_CImGuiConsole* self, bool* bInteractingWithTextbox)
{
    CONCAST(self)->display(bInteractingWithTextbox);
}

void ULog_addMessageToLog(const char* message, const ULog_LogType type)
{
    // Constructing an std::string from a null pointer is undefined behaviour; substitute a placeholder instead.
    ULog::ImGuiConsole::addToMessageLog(message != nullptr ? message : "(null)", type);
}

void ULog_addCommand(const char* cmd, const char* cmdHint, ULog_ImGuiConsole_CommandFunc func)
{
    // Constructing an std::string from a null pointer is undefined behaviour, and a null callback would crash when the
    // command is later matched, so reject an unusable registration outright rather than store a landmine.
    if (cmd == nullptr || cmdHint == nullptr || func == nullptr)
        return;
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