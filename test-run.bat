echo off
cls

rem INCHIDEREA PROCESULUI PRECEDENT ##############

taskkill /f /im testSpacedefense.exe
echo Compiling...

rem COMPILAREA PROIECTULUI #######################

g++ src/tester.cpp -o bin/testSpacedefense.exe -I "C:\SFML-2.6.1\include" -L "C:\SFML-2.6.1\lib" -lsfml-graphics -lsfml-window -lsfml-system -lsfml-audio -lsfml-network

rem EXECUTAREA PROIECTULUI #######################

if %ERRORLEVEL% EQU 0 (
    echo Compilation successful
    echo Running...
    bin\testSpacedefense.exe
) else (
    echo Compilation failed
    pause
)

