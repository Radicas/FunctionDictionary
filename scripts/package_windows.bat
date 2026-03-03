@echo off
setlocal enabledelayedexpansion

REM FunctionDictionary Windows Package Script
REM Author: FunctionDictionary Team
REM Date: 2026-03-03
REM Version: 1.0.0

set "SCRIPT_DIR=%~dp0"
set "PROJECT_ROOT=%SCRIPT_DIR%.."
cd /d "%PROJECT_ROOT%"
for %%i in ("%CD%") do set "PROJECT_ROOT=%%~fi"

REM Default configuration
set "BUILD_CONFIG=Release"
set "QT_PATH="
set "OUTPUT_DIR=%PROJECT_ROOT%\dist"
set "PACKAGE_NAME=FunctionDictionary"

REM Parse command line arguments
:parse_args
if "%~1"=="" goto :end_parse
if /i "%~1"=="-c" (set "BUILD_CONFIG=%~2" & shift & shift & goto :parse_args)
if /i "%~1"=="--config" (set "BUILD_CONFIG=%~2" & shift & shift & goto :parse_args)
if /i "%~1"=="-q" (set "QT_PATH=%~2" & shift & shift & goto :parse_args)
if /i "%~1"=="--qt-path" (set "QT_PATH=%~2" & shift & shift & goto :parse_args)
if /i "%~1"=="-o" (set "OUTPUT_DIR=%~2" & shift & shift & goto :parse_args)
if /i "%~1"=="--output" (set "OUTPUT_DIR=%~2" & shift & shift & goto :parse_args)
if /i "%~1"=="-h" goto :show_help
if /i "%~1"=="--help" goto :show_help
echo [ERROR] Unknown parameter: %~1
goto :show_help
:end_parse

echo.
echo ========================================
echo   FunctionDictionary Windows Package
echo ========================================
echo.

REM Check build configuration
if /i "%BUILD_CONFIG%"=="Debug" (
    set "BUILD_DIR=%PROJECT_ROOT%\build\src\app\Debug"
) else if /i "%BUILD_CONFIG%"=="Release" (
    set "BUILD_DIR=%PROJECT_ROOT%\build\src\app\Release"
) else (
    echo [ERROR] Invalid build configuration: %BUILD_CONFIG%
    echo        Please use Debug or Release
    exit /b 1
)

REM Check if executable exists
set "EXE_FILE=%BUILD_DIR%\FunctionDB.exe"
if not exist "%EXE_FILE%" (
    echo [ERROR] Executable not found: %EXE_FILE%
    echo.
    echo Please build the project first:
    echo   msbuild build\FunctionDictionary.sln /p:Configuration=%BUILD_CONFIG% /m
    exit /b 1
)

echo [INFO] Found executable: %EXE_FILE%

REM Find Qt installation path
if "%QT_PATH%"=="" (
    if defined Qt6_DIR (
        for %%i in ("!Qt6_DIR!") do set "QT_PATH=%%~dpi..\..\.."
    )
)

if "%QT_PATH%"=="" (
    for %%p in (
        "C:\Qt\6.10.2\msvc2022_64"
        "C:\Qt\6.9.0\msvc2022_64"
        "C:\Qt\6.8.1\msvc2022_64"
        "C:\Qt\6.8.0\msvc2022_64"
        "C:\Qt\6.7.2\msvc2019_64"
        "C:\Qt\6.7.1\msvc2019_64"
        "C:\Qt\6.6.3\msvc2019_64"
        "D:\Qt\6.10.2\msvc2022_64"
    ) do (
        if exist %%p (
            set "QT_PATH=%%~p"
            goto :qt_found
        )
    )
)

:qt_found
if "%QT_PATH%"=="" (
    echo [ERROR] Qt installation not found
    echo.
    echo Please specify Qt path using:
    echo   package_windows.bat -q "C:\Qt\6.x.x\msvc2022_64"
    exit /b 1
)

echo [INFO] Qt path: %QT_PATH%

REM Check windeployqt
set "WINDEPLOYQT=%QT_PATH%\bin\windeployqt.exe"
if not exist "%WINDEPLOYQT%" (
    echo [ERROR] windeployqt not found: %WINDEPLOYQT%
    exit /b 1
)

echo [INFO] windeployqt: %WINDEPLOYQT%

REM Add Qt bin to PATH (required by windeployqt)
set "PATH=%QT_PATH%\bin;%PATH%"

REM Create package directory
for /f "tokens=1-3 delims=/ " %%a in ('date /t') do set "DATE_STR=%%a%%b%%c"
for /f "tokens=1-3 delims=:." %%a in ('echo %time%') do set "TIME_STR=%%a%%b%%c"
set "TIMESTAMP=%DATE_STR%_%TIME_STR%"
set "TIMESTAMP=%TIMESTAMP: =0%"
set "PACKAGE_DIR=%OUTPUT_DIR%\%PACKAGE_NAME%_%BUILD_CONFIG%_%TIMESTAMP%"

echo [INFO] Creating package directory: %PACKAGE_DIR%
if not exist "%PACKAGE_DIR%" mkdir "%PACKAGE_DIR%"

REM Copy executable
echo [INFO] Copying executable...
copy /y "%EXE_FILE%" "%PACKAGE_DIR%\" >nul
if errorlevel 1 (
    echo [ERROR] Failed to copy executable
    exit /b 1
)

REM Run windeployqt
echo [INFO] Collecting Qt dependencies...
cd /d "%PACKAGE_DIR%"

if /i "%BUILD_CONFIG%"=="Release" (
    "%WINDEPLOYQT%" --release --no-translations --no-system-d3d-compiler --no-opengl-sw FunctionDB.exe
) else (
    "%WINDEPLOYQT%" --debug --no-translations --no-system-d3d-compiler --no-opengl-sw FunctionDB.exe
)
if errorlevel 1 (
    echo [WARNING] windeployqt may have issues, continuing...
)

REM Create launcher script
echo [INFO] Creating launcher script...
(
echo @echo off
echo start "" "FunctionDB.exe"
) > "%PACKAGE_DIR%\Launch.bat"

REM Create readme
echo [INFO] Creating readme...
(
echo ========================================
echo   FunctionDictionary
echo ========================================
echo.
echo Version: 1.0.0
echo Build: %BUILD_CONFIG%
echo.
echo [How to use]
echo   Double-click "Launch.bat" or "FunctionDB.exe"
echo.
echo [System Requirements]
echo   - Windows 10 or later (64-bit)
echo   - If the program fails to start, install VC++ Redistributable:
echo     https://aka.ms/vs/17/release/vc_redist.x64.exe
echo.
echo [Data Location]
echo   Database and logs are stored in:
echo   %%APPDATA%%\FunctionDictionary\
echo.
echo ========================================
) > "%PACKAGE_DIR%\README.txt"

REM Create ZIP package
echo [INFO] Creating ZIP package...
cd /d "%OUTPUT_DIR%"

set "ZIP_FILE=%PACKAGE_NAME%_%BUILD_CONFIG%_%TIMESTAMP%.zip"
set "ZIP_FILE=%ZIP_FILE: =%"

powershell -Command "Compress-Archive -Path '%PACKAGE_DIR%' -DestinationPath '%OUTPUT_DIR%\%ZIP_FILE%' -Force"
if errorlevel 1 (
    echo [ERROR] Failed to create ZIP package
    exit /b 1
)

REM Get file size
for %%f in ("%OUTPUT_DIR%\%ZIP_FILE%") do set "ZIP_SIZE=%%~zf"
set /a "ZIP_SIZE_MB=%ZIP_SIZE% / 1048576"

echo.
echo ========================================
echo   Package Complete!
echo ========================================
echo.
echo [INFO] Package details:
echo   Build config: %BUILD_CONFIG%
echo   Package dir: %PACKAGE_DIR%
echo   ZIP file: %OUTPUT_DIR%\%ZIP_FILE%
echo   File size: %ZIP_SIZE_MB% MB
echo.
echo [INFO] Instructions for customers:
echo   1. Extract %ZIP_FILE%
echo   2. Run "Launch.bat" or "FunctionDB.exe"
echo.

cd /d "%PROJECT_ROOT%"
endlocal
exit /b 0

:show_help
echo.
echo Windows Package Script
echo.
echo Usage: %~nx0 [options]
echo.
echo Options:
echo   -c, --config    Build configuration (Debug/Release, default: Release)
echo   -q, --qt-path   Qt installation path
echo   -o, --output    Output directory (default: dist)
echo   -h, --help      Show this help
echo.
echo Examples:
echo   %~nx0                              # Release package
echo   %~nx0 -c Debug                     # Debug package
echo   %~nx0 -q "C:\Qt\6.10.2\msvc2022_64"  # Specify Qt path
echo.
endlocal
exit /b 0
