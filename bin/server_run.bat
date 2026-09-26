@echo off
setlocal
set "AUTO_RUN=1"
chcp 65001 >nul

set "REPO_DIR=%~dp0"
pushd "%REPO_DIR%"
if errorlevel 1 goto :fail_no_directory

if "%AUTO_RUN%"=="0" goto :auto_run_valid
if "%AUTO_RUN%"=="1" goto :auto_run_valid
echo AUTO_RUN must be set to 0 or 1. 1>&2
goto :fail

:auto_run_valid
if not exist "%REPO_DIR%bin\driver.exe" (
    goto :missing_driver
)

if not exist "%REPO_DIR%config.tmi2" (
    echo Config file not found: "%REPO_DIR%config.tmi2"
    goto :fail
)

:run_driver
echo Starting FluffOS with config.tmi2...
"%REPO_DIR%bin\driver.exe" "%REPO_DIR%config.tmi2"
set "DRIVER_EXIT_CODE=%ERRORLEVEL%"

echo Driver exited with code %DRIVER_EXIT_CODE%.
if "%AUTO_RUN%"=="0" goto :stop
echo Restarting FluffOS in 1 minute. Press Ctrl+C to stop.
timeout /t 60 /nobreak >nul
goto :run_driver

:stop
pause
popd
exit /b %DRIVER_EXIT_CODE%

:fail
echo Server could not be started. 1>&2
pause
popd
exit /b 1

:fail_no_directory
echo Could not access the server directory: "%REPO_DIR%" 1>&2
pause
exit /b 1

:missing_driver
echo driver.exe 파일이 없습니다.
pause
popd
exit /b 1
