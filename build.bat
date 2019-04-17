@ECHO OFF && PUSHD "%~dp0" && SETLOCAL

SET "VS_TARGET=Visual Studio 14 2015"
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
ECHO "            --win64             | -64      ---   Compile in 64-bit mode.\n"
ECHO "        Make flags:\n"
ECHO "            --verbose           | -v       ---   Run make with verbose set on.\n"
ECHO "    /--------------\\ \n    |   Warnings   |\n    \\--------------/\n\n"
ECHO "        - For now build is only possible via Visual Studio 2015 of the Visual Studio versions.\n"
ECHO "        - This build script is only for those using Visual Studio, MinGW users will need to use\n"
ECHO "          cmake manually.\n"
ECHO "\n"
EXIT /B 0

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

:Win64
SET "VS_TARGET=Visual Studio 14 2015 Win64"
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
    ) ELSE IF "%1"=="-64" (
        GOTO Win64
    ) ELSE IF "%1"=="--win64" (
        GOTO Win64
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

SET "CMAKE_COMMAND=cmake -G %VS_TARGERT% %CMAKE_PARAMS% ../"
%CMAKE_COMMAND%

SET "BUILD_COMMAND=cmake --build . %BUILD_PARAMS%"
%BUILD_COMMAND%

CD ../
