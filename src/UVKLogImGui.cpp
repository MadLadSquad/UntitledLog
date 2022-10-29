#include "UVKLogImGui.h"
#ifdef UVK_LOG_IMGUI
#include "cpp/imgui_stdlib.h"

void UVKLog::ImGuiConsole::setLogColour(ImVec4 colour, LogType type)
{
    switch (type)
    {
    case UVK_LOG_TYPE_WARNING:
        warning = colour;
        return;
    case UVK_LOG_TYPE_ERROR:
        error = colour;
        return;
    case UVK_LOG_TYPE_NOTE:
        note = colour;
        return;
    case UVK_LOG_TYPE_SUCCESS:
        success = colour;
        return;
    case UVK_LOG_TYPE_MESSAGE:
        message = colour;
        return;
    }
}

void UVKLog::ImGuiConsole::display(bool* bInteractingWithTextbox)
{
    for (auto& a : loggerInternal.messageLog)
    {
        ImVec4 colour;
        switch (a.second)
        {
        case UVK_LOG_TYPE_WARNING:
            colour = warning;
            break;
        case UVK_LOG_TYPE_ERROR:
            colour = error;
            break;
        case UVK_LOG_TYPE_NOTE:
            colour = note;
            break;
        case UVK_LOG_TYPE_SUCCESS:
            colour = success;
            break;
        case UVK_LOG_TYPE_MESSAGE:
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
        for (auto& a : loggerInternal.commands)
        {
            if (a.cmd == command)
            {
                a.func();
                break;
            }
        }
        command.clear();
    }
    if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
        ImGui::SetScrollHereY(1.0f);
}

void UVKLog::ImGuiConsole::addToMessageLog(const std::string& msg, LogType type)
{
    loggerInternal.messageLog.emplace_back(msg, type);
}

void UVKLog::ImGuiConsole::addCommand(const CommandType& cmd)
{
    loggerInternal.commands.emplace_back(cmd);
}

void UVKLog::ImGuiConsole::showHelpMessage()
{
    for (const auto& a : loggerInternal.commands)
    {
        addToMessageLog(std::string(a.cmd) + " - " + a.cmdHint, UVK_LOG_TYPE_MESSAGE);
    }
}

void UVKLog::ImGuiConsole::displayFull(bool& bOpen, bool* bInteractingWithTextbox)
{
    ImGui::Begin("Developer Console", &bOpen);
    display(bInteractingWithTextbox);
    ImGui::End();
}
#endif
