@ECHO OFF
IF DEFINED VORB_PATH ( GOTO PATH_ENV )
IF EXIST Vorb\ ( GOTO PATH_SAME )
IF EXIST ..\Vorb\ ( GOTO PATH_ROOT )
IF EXIST deps\Vorb\ ( GOTO PATH_DEPS )

ECHO No path for Vorb found, please add to the environment variables as "VORB_PATH"
PAUSE
EXIT 1

:PATH_ENV
ECHO Using VORB_PATH environment variable
GOTO VORB_COPY

:PATH_SAME
ECHO Vorb was found in the same directory
SET VORB_PATH=Vorb
GOTO VORB_COPY

:PATH_ROOT
ECHO Vorb was found in the parent directory
SET VORB_PATH=..\Vorb
GOTO VORB_COPY

:PATH_DEPS
ECHO Vorb was found in the deps directory
SET VORB_PATH=deps\Vorb
GOTO VORB_COPY

:VORB_COPY
PAUSE
ROBOCOPY /E "%VORB_PATH%\include" "deps\include\Vorb" *
ROBOCOPY /E "%VORB_PATH%\deps\lib\Win32" "deps\lib\Win32" *
ROBOCOPY /E "%VORB_PATH%\deps\lib\x64" "deps\lib\x64" *
COPY  /Y "%VORB_PATH%\bin\Win32\Release\Vorb.lib " "deps\lib\Win32\Vorb.lib"
COPY  /Y "%VORB_PATH%\bin\Win32\Debug\Vorb-d.lib " "deps\lib\Win32\Vorb-d.lib"
COPY  /Y "%VORB_PATH%\bin\x64\Release\Vorb.lib "   "deps\lib\x64\Vorb.lib"
COPY  /Y "%VORB_PATH%\bin\x64\Debug\Vorb-d.lib "   "deps\lib\x64\Vorb-d.lib"
EXIT 0