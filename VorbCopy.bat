@ECHO OFF
COPY  /Y "Vorb\bin\Win32\Release\Vorb.lib " "deps\lib\Win32\Vorb.lib"
COPY  /Y "Vorb\bin\Win32\Debug\Vorb-d.lib " "deps\lib\Win32\Vorb-d.lib"
COPY  /Y "Vorb\bin\x64\Release\Vorb.lib "   "deps\lib\x64\Vorb.lib"
COPY  /Y "Vorb\bin\x64\Debug\Vorb-d.lib "   "deps\lib\x64\Vorb-d.lib"
ROBOCOPY /E /XO "Vorb\include" "deps\include\Vorb" *
ROBOCOPY /E /XO "Vorb\deps\lib\Win32" "deps\lib\Win32" *
ROBOCOPY /E /XO "Vorb\deps\lib\x64" "deps\lib\x64" *