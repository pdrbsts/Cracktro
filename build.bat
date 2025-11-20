@echo off
echo Compiling Cracktro with MSVC...
call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"

echo Compiling resources...
rc /fo resources.res resources.rc

echo Compiling mod_player.c...
cl /c /O2 mod_player.c

echo Compiling and linking main application...
cl /EHsc main.cpp effects.cpp mod_player.obj resources.res user32.lib gdi32.lib gdiplus.lib winmm.lib ole32.lib /Fe:cracktro.exe

if %errorlevel% neq 0 (
    echo Compilation failed!
    pause
    exit /b %errorlevel%
)
echo Compilation successful! Run cracktro.exe to see the demo.
