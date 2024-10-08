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
    for (const auto& a : logger.messageLog)
    {
        ImVec4 colour;
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

    static std::string command;
    if ((ImGui::InputTextWithHint("##Input", "Enter any command here", &command) || ImGui::IsItemActive()) && bInteractingWithTextbox != nullptr)
        *bInteractingWithTextbox = true;
    ImGui::SameLine();
    if (ImGui::Button("Send##consoleCommand"))
    {
        for (const auto& a : logger.commands)
        {
            if (command.rfind(a.cmd, 0) == 0)
            {
                a.func(command);
                break;
            }
        }
        command.clear();
    }
    if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
        ImGui::SetScrollHereY(1.0f);
}

void ULog::ImGuiConsole::addToMessageLog(const std::string& msg, LogType type) noexcept
{
    LoggerInternal::get().messageLog.emplace_back(msg, type);
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
    ImGui::Begin("Developer Console", &bOpen);
    display(bInteractingWithTextbox);
    ImGui::End();
}
#endif
