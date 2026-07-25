# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

**UntitledLog** — a MadLadSquad logging micro-library: colour-coded terminal/file logging with variadic arguments, a small profiling `Timer`, and an optional dear imgui developer console widget. Eight source files, no mandatory dependencies beyond libc++.

End-user documentation lives on the [GitHub wiki](https://github.com/MadLadSquad/UntitledLog/wiki), not in this repo.

Normally consumed as a git submodule. Its main consumer is UntitledImGuiFramework (`Framework/ThirdParty/logger`), whose `Framework/Core/Types.hpp` does `using namespace ULog;` framework-wide, so every `Logger::log` call in that codebase resolves here.

## Layout

- `ULogCommon.h` — the shared C ABI surface: `MLS_PUBLIC_API` cascade, `ULog_LogType` / `ULog_LogColour` / `ULog_LogOperations` enums, and the POD `ULog_Timer`. Included by both the C++ and C headers; must stay valid C.
- `ULog.hpp` / `ULog.cpp` — C++ API in `namespace ULog`. `Logger` (static-only facade), `LoggerInternal` (the singleton holding all state), `Timer`.
- `ULogImGui.hpp` / `ULogImGui.cpp` — `ULog::ImGuiConsole`, entirely inside `#ifdef ULOG_IMGUI`.
- `C/CULog.{h,cpp}`, `C/CULogImGui.{h,cpp}` — `extern "C"` wrappers. Anything added to the C++ API should get a matching `ULog_<Class>_<method>` entry point.

## Building & verifying

There is **no build system in this repo** — `CMakeLists.txt` is in `.gitignore` on purpose, as is `main.cpp`. Those names are reserved for untracked local scratch harnesses; use them rather than adding new files to the repo.

Consumers compile the `.cpp` files directly into their own target. UntitledImGuiFramework picks them up via a `GLOB_RECURSE` over `Framework/ThirdParty/logger/*.cpp` in `Framework/cmake/SetupSources.cmake` (recursive, so `C/` is included too).

There are no tests. To check a change compiles cleanly on Linux:

```bash
g++ -std=c++20 -Wall -Wextra -c ULog.cpp C/CULog.cpp
```

C++20 is required (designated initialisers in `LoggerInternal`'s ctor); the rest is C++17. The ImGui sources cannot be compiled standalone — they need `imgui.h` and `misc/cpp/imgui_stdlib.h` on the include path, plus `-DULOG_IMGUI`. Win32 branches (`_WIN32` in `getCurrentTime`, the `vasprintf` polyfill) need MSVC and cannot be verified locally; read them carefully instead.

## Preprocessor contract

Consumers control the build through four macros; the library never defines them itself.

- `ULOG_IMGUI` — compiles in the whole ImGui console (headers, sources, and the built-in `clear`/`help` commands registered in `LoggerInternal`'s constructor). **UntitledImGuiFramework does not currently define it**, so the console is dead code in that build.
- `ULOG_NO_INSTANT_CRASH` — makes `handleError` block on `std::cin.get()` before `std::terminate()`.
- `MLS_EXPORT_LIBRARY` / `MLS_LIB_COMPILE` — the shared MadLadSquad dllexport/dllimport pair, same semantics as in urll. Without `MLS_EXPORT_LIBRARY`, `MLS_PUBLIC_API` expands to nothing everywhere.

## Invariants worth preserving

These are deliberate and easy to break:

- **The C++ classes are layout-compatible hulls over their C structs.** `ULog::Timer` ↔ `ULog_Timer` and `ULog::ImGuiConsole` ↔ `ULog_CImGuiConsole` are `reinterpret_cast` between in the C wrappers, guarded by `static_assert`s in `C/CULog.cpp` and `C/CULogImGui.cpp`. Adding a member or a virtual function to either class breaks the ABI. This is why the console's input-box buffer (`consoleCommand`) lives on `LoggerInternal` instead of on `ImGuiConsole`.
- **There are two parallel implementations of the log pipeline** — the templated `LoggerInternal::agnostic<bTerminal, bFile>` (C++, `<<`-based) and `ULog_Logger_logV` (C, `vasprintf`-based). A change to the timestamp/prefix format, stream dispatch, or error handling must be mirrored in both. Both must end by calling `pushMessage()` and then `handleError()`, in that order, so a fatal error still reaches the file.
- **`logColours` is one array serving two purposes**: indices 0–5 are ANSI escape codes, indices `type + logTypeOffset` are the human-readable type names, and `logTypeOffset - 1` is the reset code. Every index derived from a `LogType` must pass through `sanitizeLogType()` first — the enum arrives from C callers and is not range-checked by the type system.
- **`messageLog` entries are single physical lines, not log calls.** `pushMessage` splits on `'\n'` because the console renders through an `ImGuiListClipper`, which assumes uniform row height. Consequently `maxLogMessages` bounds lines; `0` means unbounded.
- **`LoggerInternal::get(lg)` is a one-shot injection point.** The first call decides the singleton — passing a pointer adopts an existing instance, passing `nullptr` creates one; later arguments are ignored. UntitledImGuiFramework relies on this to share one logger across shared-library boundaries (`FrameworkMain::setupGlobal` and `Utility::loadContext` inject it; `PluginInterface` hands it out).
- **`bLoggingEnabled == false` must suppress termination too.** `handleError` deliberately mirrors `agnostic`'s early-out — otherwise a silenced logger would still crash the app on an error with nothing written anywhere.
- **The C boundary null-checks every pointer it forwards** (`fmt`, `message`, `file`, command strings and callbacks), because constructing `std::string`/`std::ofstream` from `nullptr` is UB. Keep new C entry points equally defensive.
- Every public C++ function is `noexcept`, and every declaration carries a `// UntitledImGuiFramework Event Safety - Any time` comment. That tag is a convention inherited from the parent framework; new declarations should carry it too.
- `Timer::~Timer` intentionally does **not** call `stop()`.

## Releases

Pushing a `v*` tag triggers `.github/workflows/release.yaml`, which strips `.git/`, tars the tree as `untitled-log-<version>.tar.xz`, and publishes a GitHub release. Versions are four-part (`v6.1.0.0`).

## graphify

This project has a knowledge graph at graphify-out/ with god nodes, community structure, and cross-file relationships.

Rules:
- For codebase questions, first run `graphify query "<question>"` when graphify-out/graph.json exists. Use `graphify path "<A>" "<B>"` for relationships and `graphify explain "<concept>"` for focused concepts. These return a scoped subgraph, usually much smaller than GRAPH_REPORT.md or raw grep output.
- If graphify-out/wiki/index.md exists, use it for broad navigation instead of raw source browsing.
- Read graphify-out/GRAPH_REPORT.md only for broad architecture review or when query/path/explain do not surface enough context.
- After modifying code, run `graphify update .` to keep the graph current (AST-only, no API cost).
