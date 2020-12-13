#pragma once
#include <iostream>
#include <fstream>
#include <ostream>

#define LogColGreen  "\x1B[32m"
#define LogColYellow "\x1B[33m"
#define LogColRed "\x1B[31m"
#define LogColWhite  "\x1B[37m"
#define LogColBlue  "\x1B[34m"

enum LogType
{
    WARNING,
    ERROR,
    NOTE,
    SUCCESS,
    MESSAGE
};

class UVKLog
{
public:
    template<typename T>
    void ConsoleLog(const char* message, LogType type, std::initializer_list<T> list)
    {
        switch (type)
        {
            case SUCCESS:
                std::cout << LogColGreen << "Message: " << message;
                for (auto elem : list)
                {
                    std::cout << " " << elem;
                }
                std::cout << std::endl;
                break;
            case WARNING:
                std::cout << LogColYellow << "Message: " << message;
                for (auto elem : list)
                {
                    std::cout << " " << elem;
                }
                std::cout << std::endl;
                break;
            case ERROR:
                std::cout << LogColRed << "Message: " << message;
                for (auto elem : list)
                {
                    std::cout << " " << elem;
                }
                std::cout << std::endl;
                break;
            case NOTE:
                std::cout << LogColBlue << "Message: " << message;
                for (auto elem : list)
                {
                    std::cout << " " << elem;
                }
                std::cout << std::endl;
                break;
            case MESSAGE:
                std::cout << LogColWhite << "Message: " << message;
                for (auto elem : list)
                {
                    std::cout << " " << elem;
                }
                std::cout << std::endl;
                break;
        }
    }

    void FileLog(const char* message, LogType type, std::ofstream file, ...)
    {
        //file << message << ... << std::endl;
    }
};
