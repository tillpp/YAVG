@echo off
setlocal EnableDelayedExpansion

:: ============================================================
:: Self-Contained C++ CMake Build Script for Windows
:: Downloads portable WinLibs and CMake binaries
:: ============================================================

set "SCRIPT_DIR=%~dp0"
IF %SCRIPT_DIR:~-1%==\ SET SCRIPT_DIR=%SCRIPT_DIR:~0,-1%
set "TOOLS_DIR=%SCRIPT_DIR%\tools"
set "BUILD_DIR=%SCRIPT_DIR%\build"

:: ---- Configurable versions --------------------------------
set "GCC_VERSION=15.2.0"
set "MINGW_VERSION=14.0.0"
set "WINLIBS_RELEASE=7"
set "CMAKE_VERSION=4.0.3"
:: -----------------------------------------------------------

:: Derived paths
set "WINLIBS_ZIP=winlibs-x86_64-posix-seh-gcc-%GCC_VERSION%-mingw-w64ucrt-%MINGW_VERSION%-r%WINLIBS_RELEASE%.zip"
set "WINLIBS_URL=https://github.com/brechtsanders/winlibs_mingw/releases/download/%GCC_VERSION%posix-%MINGW_VERSION%-ucrt-r%WINLIBS_RELEASE%/%WINLIBS_ZIP%"
set "MINGW_DIR=%TOOLS_DIR%\mingw64"

set "CMAKE_ZIP=cmake-%CMAKE_VERSION%-windows-x86_64.zip"
set "CMAKE_URL=https://github.com/Kitware/CMake/releases/download/v%CMAKE_VERSION%/%CMAKE_ZIP%"
set "CMAKE_DIR=%TOOLS_DIR%\cmake-%CMAKE_VERSION%-windows-x86_64"

:: ---- Detect available download tool -----------------------
set "DOWNLOAD_TOOL="
where curl >nul 2>&1 && set "DOWNLOAD_TOOL=curl"
if not defined DOWNLOAD_TOOL (
    where powershell >nul 2>&1 && set "DOWNLOAD_TOOL=powershell"
)
if not defined DOWNLOAD_TOOL (
    echo ERROR: Neither curl nor powershell found. Cannot download dependencies.
    exit /b 1
)
:: -----------------------------------------------------------

echo ==========================================================
echo  Self-Contained C++ CMake Build
echo ==========================================================
echo.

:: ---- Create tools directory --------------------------------
if not exist "%TOOLS_DIR%" mkdir "%TOOLS_DIR%"

:: ============================================================
:: Download and extract WinLibs
:: ============================================================
if not exist "%MINGW_DIR%\bin\gcc.exe" (
    echo [1/4] Downloading WinLibs GCC %GCC_VERSION%
    echo       URL: %WINLIBS_URL%
    echo.

    if "%DOWNLOAD_TOOL%"=="curl" (
        curl -L --progress-bar -o "%TOOLS_DIR%\%WINLIBS_ZIP%" "%WINLIBS_URL%"
    ) else (
        powershell -NoProfile -Command ^
          "Write-Host 'Downloading...'; " ^
          "[Net.ServicePointManager]::SecurityProtocol = [Net.SecurityProtocolType]::Tls12; " ^
          "Invoke-WebRequest -Uri '%WINLIBS_URL%' -OutFile '%TOOLS_DIR%\%WINLIBS_ZIP%' -UseBasicParsing"
    )

    if errorlevel 1 (
        echo ERROR: Failed to download WinLibs
        if exist "%TOOLS_DIR%\%WINLIBS_ZIP%" del /f /q "%TOOLS_DIR%\%WINLIBS_ZIP%"
        exit /b 1
    )

    if not exist "%TOOLS_DIR%\%WINLIBS_ZIP%" (
        echo ERROR: WinLibs archive was not downloaded
        exit /b 1
    )

    echo [2/4] Extracting WinLibs

    where tar >nul 2>&1
    if not errorlevel 1 (
        tar -xf "%TOOLS_DIR%\%WINLIBS_ZIP%" -C "%TOOLS_DIR%"
    ) else (
        powershell -NoProfile -Command ^
          "Add-Type -AssemblyName System.IO.Compression.FileSystem; [System.IO.Compression.ZipFile]::ExtractToDirectory('%TOOLS_DIR%\%WINLIBS_ZIP%', '%TOOLS_DIR%')"
    )

    if errorlevel 1 (
        echo ERROR: Failed to extract WinLibs archive
        echo The archive may be incomplete or PowerShell may not support this ZIP reliably
        echo Try deleting "%TOOLS_DIR%\%WINLIBS_ZIP%" and rerunning
        exit /b 1
    )

    del /f /q "%TOOLS_DIR%\%WINLIBS_ZIP%"
    echo       Done.
) else (
    echo [1/4] WinLibs GCC already present, skipping download
    echo [2/4] Skipped (already extracted)
)

:: ============================================================
:: Download and extract portable CMake
:: ============================================================
if not exist "%CMAKE_DIR%\bin\cmake.exe" (
    echo [3/4] Downloading CMake %CMAKE_VERSION%
    echo       URL: %CMAKE_URL%
    echo.

    if "%DOWNLOAD_TOOL%"=="curl" (
        curl -L --progress-bar -o "%TOOLS_DIR%\%CMAKE_ZIP%" "%CMAKE_URL%"
    ) else (
        powershell -NoProfile -Command ^
          "Write-Host 'Downloading...'; " ^
          "[Net.ServicePointManager]::SecurityProtocol = [Net.SecurityProtocolType]::Tls12; " ^
          "Invoke-WebRequest -Uri '%CMAKE_URL%' -OutFile '%TOOLS_DIR%\%CMAKE_ZIP%' -UseBasicParsing"
    )

    if errorlevel 1 (
        echo ERROR: Failed to download CMake
        if exist "%TOOLS_DIR%\%CMAKE_ZIP%" del /f /q "%TOOLS_DIR%\%CMAKE_ZIP%"
        exit /b 1
    )

    if not exist "%TOOLS_DIR%\%CMAKE_ZIP%" (
        echo ERROR: CMake archive was not downloaded
        exit /b 1
    )

    echo [4/4] Extracting CMake...
    where tar >nul 2>&1
    if not errorlevel 1 (
        tar -xf "%TOOLS_DIR%\%CMAKE_ZIP%" -C "%TOOLS_DIR%"
    ) else (
        powershell -NoProfile -Command ^
          "Add-Type -AssemblyName System.IO.Compression.FileSystem; [System.IO.Compression.ZipFile]::ExtractToDirectory('%TOOLS_DIR%\%CMAKE_ZIP%', '%TOOLS_DIR%')"
    )

    if errorlevel 1 (
        echo ERROR: Failed to extract CMake archive
        echo The archive may be incomplete or PowerShell may not support this ZIP reliably
        echo Try deleting "%TOOLS_DIR%\%CMAKE_ZIP%" and rerunning
        exit /b 1
    )

    del "%TOOLS_DIR%\%CMAKE_ZIP%"
    echo       Done
) else (
    echo [3/4] CMake already present, skipping download
    echo [4/4] Skipped (already extracted)
)

:: ============================================================
:: Add tools to $PATH
:: ============================================================
set "PATH=%MINGW_DIR%\bin;%CMAKE_DIR%\bin;%PATH%"

echo.
echo --- Toolchain Versions ---
gcc --version | findstr gcc
cmake --version | findstr cmake
echo --------------------------
echo.

:: ============================================================
:: Configure with CMake using MinGW Makefiles generator
:: ============================================================
echo Configuring project with CMake
if not exist "%BUILD_DIR%" mkdir "%BUILD_DIR%"

cmake -S "%SCRIPT_DIR%" ^
      -B "%BUILD_DIR%" ^
      -G "MinGW Makefiles" ^
      -DCMAKE_C_COMPILER="%MINGW_DIR%\bin\gcc.exe" ^
      -DCMAKE_CXX_COMPILER="%MINGW_DIR%\bin\g++.exe" ^
      -DCMAKE_MAKE_PROGRAM="%MINGW_DIR%\bin\mingw32-make.exe" ^
      -DCMAKE_BUILD_TYPE=Release

if errorlevel 1 (
    echo.
    echo ERROR: CMake configuration failed
    echo Make sure a CMakeLists.txt exists in: %SCRIPT_DIR%
    exit /b 1
)

:: ============================================================
:: Build
:: ============================================================
echo.
echo Building project...
cmake --build "%BUILD_DIR%" --config Release -- -j%NUMBER_OF_PROCESSORS%

if errorlevel 1 (
    echo.
    echo ERROR: Build failed. See output above for details
    exit /b 1
)

echo.
echo ==========================================================
echo  Build complete!
echo  Output is in: %BUILD_DIR%
echo ==========================================================
echo.

endlocal
