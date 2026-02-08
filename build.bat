@echo off
setlocal EnableExtensions EnableDelayedExpansion

set "PROJECT_ROOT=%~dp0"
set "BUILD_DIR=%PROJECT_ROOT%build"
set "BUILD_TYPE=Release"
set "RUN_AFTER_BUILD=0"
set "CLEAN=0"

if /I "%~1"=="/h" goto :help
if /I "%~1"=="-h" goto :help
if /I "%~1"=="--help" goto :help

:parse
if "%~1"=="" goto :parsed
if /I "%~1"=="clean" (
  set "CLEAN=1"
) else if /I "%~1"=="debug" (
  set "BUILD_TYPE=Debug"
) else if /I "%~1"=="release" (
  set "BUILD_TYPE=Release"
) else if /I "%~1"=="run" (
  set "RUN_AFTER_BUILD=1"
) else (
  echo [build.bat] Unknown argument: %~1
  goto :help
)
shift
goto :parse

:parsed
if "%CLEAN%"=="1" (
  if exist "%BUILD_DIR%" (
    echo [build.bat] Cleaning build directory: "%BUILD_DIR%"
    rmdir /S /Q "%BUILD_DIR%"
  )
)

if not exist "%BUILD_DIR%" mkdir "%BUILD_DIR%"

echo [build.bat] Configuring with CMake ^(type=%BUILD_TYPE%^)
cmake -S "%PROJECT_ROOT%" -B "%BUILD_DIR%" -DCMAKE_BUILD_TYPE=%BUILD_TYPE%
if errorlevel 1 goto :error

echo [build.bat] Building
cmake --build "%BUILD_DIR%" --config %BUILD_TYPE%
if errorlevel 1 goto :error

if "%RUN_AFTER_BUILD%"=="1" (
  set "APP1=%BUILD_DIR%\ClassAssistant.exe"
  set "APP2=%BUILD_DIR%\%BUILD_TYPE%\ClassAssistant.exe"

  if exist "!APP1!" (
    echo [build.bat] Running: "!APP1!"
    start "" /B "!APP1!"
  ) else if exist "!APP2!" (
    echo [build.bat] Running: "!APP2!"
    start "" /B "!APP2!"
  ) else (
    echo [build.bat] Build succeeded, but executable not found automatically.
  )
)

echo [build.bat] Done.
exit /B 0

:help
echo Usage: build.bat [clean] [debug^|release] [run]
echo.
echo   clean    Remove existing build directory before configuring.
echo   debug    Build with CMAKE_BUILD_TYPE=Debug.
echo   release  Build with CMAKE_BUILD_TYPE=Release ^(default^).
echo   run      Run executable after successful build.
exit /B 0

:error
echo [build.bat] Build failed.
exit /B 1
