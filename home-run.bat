echo off
cls

rem INCHIDEREA PROCESULUI PRECEDENT ##############

taskkill /f /im spacedefense.exe >nul

if %ERRORLEVEL% EQU 0 (
    echo Previous process closed
) else (
    @REM echo Failed to close previous process
    cls
)

echo Compiling...

rem COMPILAREA PROIECTULUI #######################

g++ src/main.cpp -o bin/spacedefense.exe -I "X:\SFML-2.6.1\include" -L "X:\SFML-2.6.1\lib" -lsfml-graphics -lsfml-window -lsfml-system -lsfml-audio -lsfml-network -mwindows -static-libgcc -static-libstdc++

rem EXECUTAREA PROIECTULUI #######################

if %ERRORLEVEL% EQU 0 (
    echo Compilation successful
    echo Running...
    bin\spacedefense.exe
    echo Game closed
) else (
    echo Compilation failed
    pause
)