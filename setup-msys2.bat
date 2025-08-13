@echo off
REM setup-msys2.bat - Windows batch wrapper for Ghoul MSYS2 setup
REM This script launches the setup script in MSYS2 UCRT64 environment

echo ================================================
echo   Ghoul Language - MSYS2 UCRT64 Setup Launcher
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
echo Launching setup script in UCRT64 environment...
echo.

REM Launch the setup script in MSYS2 UCRT64
C:\msys64\msys2_shell.cmd -defterm -here -no-start -ucrt64 -shell bash -c "./setup-msys2.sh"

echo.
echo Setup completed. You can now use MSYS2 UCRT64 terminal to build Ghoul.
pause