@ECHO OFF && PUSHD "%~dp0" && SETLOCAL

REM ====================================================================
REM   Build: Clean
REM ====================================================================
IF "%~1"=="clean" (
    RMDIR build /S /Q
    shift
)

REM ====================================================================
REM   Create: Build folder
REM ====================================================================
IF NOT EXIST "build" MKDIR build

REM ====================================================================
REM   CMake: Clean any top-level cmake buid artifacts
REM ====================================================================
IF EXIST "cmake" RMDIR cmake /S /Q
IF EXIST "CMakeFiles" RMDIR CMakeFiles /S /Q
IF EXIST "CmakeCache.txt" DEL CmakeCache.txt


REM ====================================================================
REM   CMake: Begin cmake build process
REM ====================================================================
cd build

REM ====================================================================
REM   CMake: Create build files
REM ====================================================================
cmake ../
IF "%~1"=="rebuild" (
    make clean
    shift
)
REM ====================================================================
REM   CMake: Run Make
REM ====================================================================
make %*