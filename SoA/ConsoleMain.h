//
// ConsoleMain.h
// Seed of Andromeda
//
// Created by Cristian Zaloj on 29 Jun 2015
// Copyright 2014 Regrowth Studios
// MIT License
//
// Summary:
// Main entry point for the console version of the application.
//

#pragma once

#ifndef ConsoleMain_h__
#define ConsoleMain_h__

#include <Vorb/script/IEnvironment.hpp>
#include <Vorb/script/ConsoleBackend.hpp>

#include "ConsoleFuncs.h"

#ifdef VORB_OS_WINDOWS
#define SOA_CONSOLE_COLOR_HEADER (FOREGROUND_BLUE | FOREGROUND_INTENSITY)
#define SOA_CONSOLE_COLOR_PROMPT (FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY)
#define SOA_CONSOLE_COLOR_MAIN (FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE)
#define SOA_CONSOLE_COLOR_OUTPUT (FOREGROUND_GREEN | FOREGROUND_INTENSITY)
#define SOA_CONSOLE_COLOR_ERROR (FOREGROUND_RED | FOREGROUND_INTENSITY)
#else//VORB_OS_WINDOWS
#define SOA_CONSOLE_COLOR_HEADER 34
#define SOA_CONSOLE_COLOR_PROMPT 33
#define SOA_CONSOLE_COLOR_MAIN 37
#define SOA_CONSOLE_COLOR_OUTPUT 32
#define SOA_CONSOLE_COLOR_ERROR 31
#endif//VORB_OS_WINDOWS

namespace {
    struct ConsolePrinter {
    public:
        void setColor(ui32 color) {
            if (m_lastColor != color) {
                fflush(stdout);
                fflush(stderr);
            }
            m_lastColor = color;
#ifdef VORB_OS_WINDOWS
            SetConsoleTextAttribute(hndConsole, color);
#else
            printf("\033[0;%dm", color);
#endif//VORB_OS_WINDOWS            
        }
        void out(Sender, const cString msg) {
            setColor(SOA_CONSOLE_COLOR_OUTPUT);
            puts(msg);
        }
        void err(Sender, const cString msg) {
            setColor(SOA_CONSOLE_COLOR_ERROR);
            puts(msg);
        }

#ifdef VORB_OS_WINDOWS
        HANDLE hndConsole;
#endif//VORB_OS_WINDOWS

    private:
        ui32 m_lastColor = 0;
    };
}

template <typename ScriptImpl>
void consoleMain() {
    // Get console for manipulation
    ConsolePrinter printer = {};

#ifdef VORB_OS_WINDOWS
    printer.hndConsole = GetStdHandle(STD_OUTPUT_HANDLE);
#endif//VORB_OS_WINDOWS

    // Write out that we are using the console version
    printer.setColor(SOA_CONSOLE_COLOR_HEADER);
    puts(R"(
     _________________
    /                /
   /                /
  /  SoA Console   /
 /                / ______
/                /  |v0.1|
==========================
)");

    vscript::IEnvironment<ScriptImpl>* env = new ScriptImpl();

    vscript::ConsoleBackend<ScriptImpl> repl;
    repl.init(env);

    registerFuncs<ScriptImpl>(env);

    repl.onConsoleOutput.out += makeDelegate(&printer, &ConsolePrinter::out);
    repl.onConsoleOutput.err += makeDelegate(&printer, &ConsolePrinter::err);

    char buf[1024];    
    while (true) {
        printer.setColor(SOA_CONSOLE_COLOR_PROMPT);
        printf(">>> ");
        printer.setColor(SOA_CONSOLE_COLOR_MAIN);
        std::cin.getline(buf, 1024);
        repl.invokeCommand(buf);
    }

    delete env;
}

#endif // ConsoleMain_h__
