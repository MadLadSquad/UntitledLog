#include "ULogImGui.hpp"
#ifdef ULOG_IMGUI
#include "cpp/imgui_stdlib.h"

void ULog::ImGuiConsole::setLogColour(const ImVec4 colour, const LogType type) noexcept
{
    switch (type)
    {
    case ULOG_LOG_TYPE_WARNING:
        warning = colour;
        return;
    case ULOG_LOG_TYPE_ERROR:
        error = colour;
        return;
    case ULOG_LOG_TYPE_NOTE:
        note = colour;
        return;
    case ULOG_LOG_TYPE_SUCCESS:
        success = colour;
        return;
    case ULOG_LOG_TYPE_MESSAGE:
        message = colour;
        return;
    }
}

void ULog::ImGuiConsole::display(bool* bInteractingWithTextbox) const noexcept
{
    auto& logger = LoggerInternal::get();

    // Scope every widget ID below to this console instance so multiple consoles don't collide on the input box/button
    ImGui::PushID(this);
    // Only submit the rows that are actually visible. With the log at its default cap of 1000 entries this turns a
    // full-list walk every frame into a handful of TextColored calls.
    ImGuiListClipper clipper;
    clipper.Begin(static_cast<int>(logger.messageLog.size()));
    while (clipper.Step())
    {
        for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; ++i)
        {
            const auto& a = logger.messageLog[static_cast<size_t>(i)];
            ImVec4 colour = message;
            switch (a.second)
            {
            case ULOG_LOG_TYPE_WARNING:
                colour = warning;
                break;
            case ULOG_LOG_TYPE_ERROR:
                colour = error;
                break;
            case ULOG_LOG_TYPE_NOTE:
                colour = note;
                break;
            case ULOG_LOG_TYPE_SUCCESS:
                colour = success;
                break;
            case ULOG_LOG_TYPE_MESSAGE:
                colour = message;
                break;
            }

            ImGui::TextColored(colour, "%s", a.first.c_str());
        }
    }

    std::string& command = logger.consoleCommand;
    // EnterReturnsTrue lets Enter submit the command, matching the Send button - the expected interaction for a console.
    const bool bEnterPressed = ImGui::InputTextWithHint("##Input", "Enter any command here", &command,
                                                        ImGuiInputTextFlags_EnterReturnsTrue);
    if (ImGui::IsItemActive() && bInteractingWithTextbox != nullptr)
        *bInteractingWithTextbox = true;
    // Re-focus the input box after an Enter submission so successive commands can be typed without clicking back in.
    if (bEnterPressed)
        ImGui::SetKeyboardFocusHere(-1);
    ImGui::SameLine();
    if (ImGui::Button("Send##consoleCommand") || bEnterPressed)
    {
        // Match on the whole first whitespace-delimited token, not a prefix - otherwise "clearall" would fire "clear"
        // and registration order would decide ambiguous prefixes. The full command line is still passed to the handler.
        // Skip any leading whitespace first so "  clear" still resolves to the "clear" token.
        const size_t tokenStart = command.find_first_not_of(" \t");
        const std::string token = tokenStart == std::string::npos
                                      ? std::string{}
                                      : command.substr(tokenStart, command.find_first_of(" \t", tokenStart) - tokenStart);
        // Copy the matched handler out before invoking it: a handler is free to register new commands (e.g. a "load
        // module" command), which can reallocate logger.commands and destroy the very std::function we are running.
        std::function<void(const std::string&)> handler;
        for (const auto& a : logger.commands)
        {
            if (token == a.cmd)
            {
                handler = a.func;
                break;
            }
        }
        if (handler)
            handler(command);
        command.clear();
    }
    if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
        ImGui::SetScrollHereY(1.0f);
    ImGui::PopID();
}

void ULog::ImGuiConsole::addToMessageLog(const std::string& msg, LogType type) noexcept
{
    LoggerInternal::get().pushMessage(msg, type);
}

void ULog::ImGuiConsole::addCommand(const CommandType& cmd) noexcept
{
    LoggerInternal::get().commands.emplace_back(cmd);
}

void ULog::ImGuiConsole::showHelpMessage(const std::string&) noexcept
{
    for (const auto& a : LoggerInternal::get().commands)
        addToMessageLog(std::string(a.cmd) + " - " + a.cmdHint, ULOG_LOG_TYPE_MESSAGE);
}

void ULog::ImGuiConsole::displayFull(bool& bOpen, bool* bInteractingWithTextbox) const noexcept
{
    // Skip the (potentially expensive) contents when the window is collapsed, but always pair Begin with End.
    if (ImGui::Begin("Developer Console", &bOpen))
        display(bInteractingWithTextbox);
    ImGui::End();
}
#endif
