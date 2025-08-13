@echo off
REM create-bundle.bat - Run the bundle script in MSYS2 UCRT64 environment

echo ================================================
echo   Ghoul Bundle Creator - MSYS2 Launcher
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
echo Running bundle script in UCRT64 environment...
echo.

REM Launch the bundle script in MSYS2 UCRT64
C:\msys64\msys2_shell.cmd -defterm -here -no-start -ucrt64 -shell bash -c "./bundle-windows.sh"

echo.
echo Bundle creation completed.
pause