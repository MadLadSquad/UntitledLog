#pragma once
#include "ULog.hpp"
#ifdef ULOG_IMGUI
#if __has_include(<imgui.h>)
#include "imgui.h"

namespace ULog
{
    // UntitledImGuiFramework Event Safety - Any time
    class MLS_PUBLIC_API ImGuiConsole
    {
    public:
        // UntitledImGuiFramework Event Safety - Any time
        void displayFull(bool& bOpen, bool* bInteractingWithTextbox = nullptr) const noexcept;
        // UntitledImGuiFramework Event Safety - Any time
        void display(bool* bInteractingWithTextbox = nullptr) const noexcept;

        // UntitledImGuiFramework Event Safety - Any time
        static void addToMessageLog(const std::string& msg, LogType type) noexcept;

        // UntitledImGuiFramework Event Safety - Any time
        static void addCommand(const CommandType& cmd) noexcept;

        // UntitledImGuiFramework Event Safety - Any time
        void setLogColour(ImVec4 colour, LogType type) noexcept;
    private:
        friend class LoggerInternal;

        static void showHelpMessage(const std::string&) noexcept;

        ImVec4 success = { 0.0f, 1.0f, 0.0f, 1.0f };
        ImVec4 warning = { 1.0f, 1.0f, 0.0f, 1.0f };
        ImVec4 error = { 1.0f, 0.0f, 0.0f, 1.0f };
        ImVec4 note = { 0.0f, 0.0f, 1.0f, 1.0f };
        ImVec4 message = { 1.0f, 1.0f, 1.0f, 1.0f };
    };
}
#else
#error Dear ImGui is not in the include path! Consider adding it so that "#include <imgui.h>" is a valid line
#endif
#endif
