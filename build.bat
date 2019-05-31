@ECHO OFF && PUSHD "%~dp0" && SETLOCAL

SET VS_TARGET="2015"
SET BUILD_CLEAN="false"
SET "CMAKE_PARAMS="
SET "BUILD_PARAMS="

GOTO Argloop

:Help
ECHO "\n"
ECHO "    /--------------\\ \n    |  Build Help  |\n    \\--------------/\n\n"
ECHO "        Build flags:\n"
ECHO "            --clean             | -c       ---   Clean build, removes all previous artefacts.\n"
ECHO "        CMake flags:\n"
ECHO "            --release           | -r       ---   Compile in release mode.\n"
ECHO "            --debug             | -d       ---   Compile in debug mode.\n"
ECHO "            --cxx17             | -17      ---   Target C++17 (otherwise targets C++14).\n"
ECHO "            --no-gdb            | -ng      ---   Add OS specific debug symbols rather than GDB's.\n"
ECHO "            --no-extra-debug    | -ned     ---   Don't add extra debug symbols.\n"
ECHO "            --no-optimise-debug | -nod     ---   Don't optimise debug mode builds.\n"
ECHO "        Make flags:\n"
ECHO "            --verbose           | -v       ---   Run make with verbose set on.\n"
ECHO "\n"
GOTO ArgloopContinue

:Clean
SET BUILD_CLEAN="true"
SET "BUILD_PARAMS=%BUILD_PARAMS% --clean-first"
GOTO ArgloopContinue

:Release
SET "CMAKE_PARAMS=%CMAKE_PARAMS% -DCMAKE_BUILD_TYPE=Release"
GOTO ArgloopContinue

:Debug
SET "CMAKE_PARAMS=%CMAKE_PARAMS% -DCMAKE_BUILD_TYPE=Debug"
GOTO ArgloopContinue

:Cxx17
SET "CMAKE_PARAMS=%CMAKE_PARAMS% -DTARGET_CXX_17=On"
GOTO ArgloopContinue

:NoGDB
SET "CMAKE_PARAMS=%CMAKE_PARAMS% -DUSING_GDB=Off"
GOTO ArgloopContinue

:NoExtraDebug
SET "CMAKE_PARAMS=%CMAKE_PARAMS% -DEXTRA_DEBUG=Off"
GOTO ArgloopContinue

:NoOptimiseDebug
SET "CMAKE_PARAMS=%CMAKE_PARAMS% -DOPTIMISE_ON_DEBUG=Off"
GOTO ArgloopContinue

:Verbose
SET "BUILD_PARAMS=%BUILD_PARAMS% -- VERBOSE=1"
GOTO ArgloopContinue

:Argloop
    IF "%1"=="-h" (
        GOTO Help
    ) ELSE IF "%1"=="--help" (
        GOTO Help
    ) ELSE IF "%1"=="-c" (
        GOTO Clean
    ) ELSE IF "%1"=="--clean" (
        GOTO Clean
    ) ELSE IF "%1"=="-r" (
        GOTO Release
    ) ELSE IF "%1"=="--release" (
        GOTO Release
    ) ELSE IF "%1"=="-d" (
        GOTO Debug
    ) ELSE IF "%1"=="--debug" (
        GOTO Debug
    ) ELSE IF "%1"=="-17" (
        GOTO Cxx17
    ) ELSE IF "%1"=="--cxx17" (
        GOTO Cxx17
    ) ELSE IF "%1"=="-ng" (
        GOTO NoGDB
    ) ELSE IF "%1"=="--no-gdb" (
        GOTO NoGDB
    ) ELSE IF "%1"=="-ned" (
        GOTO NoExtraDebug
    ) ELSE IF "%1"=="--no-extra-debug" (
        GOTO NoExtraDebug
    ) ELSE IF "%1"=="-nod" (
        GOTO NoOptimiseDebug
    ) ELSE IF "%1"=="--no-optimise-debug" (
        GOTO NoOptimiseDebug
    ) ELSE IF "%1"=="-v" (
        GOTO Verbose
    ) ELSE IF "%1"=="--verbose" (
        GOTO Verbose
    ) ELSE IF NOT "%1"=="" (
        ECHO "Error: Do not recognise argument %1."
        EXIT /B 1
    ) ELSE (
        GOTO ArgloopBreak
    )
    :ArgloopContinue
    SHIFT
GOTO Argloop
:ArgloopBreak

IF EXIST "build" (
    IF "%BUILD_CLEAN%"=="true" (
        RMDIR build /S /Q
    )
) ELSE (
    MKDIR build
)

CD build

SET "CMAKE_COMMAND=cmake %CMAKE_PARAMS% -G "Visual Studio 14 2015 Win64" ../"
%CMAKE_COMMAND%

SET "BUILD_COMMAND=cmake --build . %MAKE_PARAMS%"
%BUILD_COMMAND%

CD ../
