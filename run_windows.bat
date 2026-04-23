@echo off
setlocal enabledelayedexpansion


REM CONFIGURATION

set ROOT=%~dp0compiler\x86_64

set LLVM_VER=22.1.4
set CMAKE_VER=3.30.5

set LLVM_URL=https://github.com/llvm/llvm-project/releases/download/llvmorg-%LLVM_VER%/clang+llvm-%LLVM_VER%-x86_64-pc-windows-msvc.tar.xz
set CMAKE_URL=https://github.com/Kitware/CMake/releases/download/v%CMAKE_VER%/cmake-%CMAKE_VER%-windows-x86_64.tar.xz

set LLVM_ZIP=%ROOT%\llvm.zip
set CMAKE_ZIP=%ROOT%\cmake.zip

REM FODLER CREATION
echo Creating %ROOT%
if not exist "%ROOT%" mkdir "%ROOT%"

REM DOWNLOAD CLANG
echo Downloading Clang and Cmake ....%LLVM_URL% into %LLVM_ZIP%
powershell -Command $ProgressPreference = 'SilentlyContinue'; "Invoke-WebRequest -uri '%LLVM_URL%' -OutFile '%LLVM_ZIP%'"
powershell -Command $ProgressPreference = 'SilentlyContinue'; "Invoke-WebRequest -uri '%CMAKE_URL%' -OutFile '%CMAKE_ZIP%'"
echo Extracting zip file...
C:\Windows\System32\tar.exe -xf %LLVM_ZIP% --directory %ROOT%\llvm
C:\Windows\System32\tar.exe -xf %CMAKE_ZIP% --directory %ROOT%\cmake


REM %CMAKE_URL% %CMAKE_ZIP%
REM powershell -Command "Invoke-WebRequest -Uri \"%LLVM_URL%\" -OutFile \"%LLVM_ZIP%\""
REM powershell -Command "Expand-Archive -Force '%LLVM_ZIP%' '%ROOT%\llvm'"
REM DOWNLOAD CMAKE
REM curl -L %CMAKE_URL% -o "%CMAKE_ZIP%"
REM powershell -Command "Expand-Archive -Force '%CMAKE_ZIP%' '%ROOT%\cmake'"