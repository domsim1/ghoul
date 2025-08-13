@echo off
REM start-msys2.bat - Quick launcher for MSYS2 UCRT64 environment
REM Opens MSYS2 UCRT64 terminal in the current project directory

echo ================================================
echo   Ghoul Language - MSYS2 UCRT64 Quick Launcher
echo ================================================
echo.

REM Check if MSYS2 is installed
if not exist "C:\msys64\msys2_shell.cmd" (
    echo ERROR: MSYS2 not found at C:\msys64\
    echo.
    echo Please install MSYS2 first:
    echo 1. Download from https://www.msys2.org/
    echo 2. Install to C:\msys64\ (default location^)
    echo 3. Run this script again
    echo.
    pause
    exit /b 1
)

echo Found MSYS2 installation
echo Opening UCRT64 terminal in project directory...
echo.

REM Launch MSYS2 UCRT64 terminal in current directory
C:\msys64\msys2_shell.cmd -defterm -here -no-start -ucrt64

echo.
echo MSYS2 terminal closed.