@echo off
setlocal

set SRC_DIR=..\src
set OUT_DIR=..\bin\x86
set DLL_NAME=ev_rclone.dll

if not exist %OUT_DIR% mkdir %OUT_DIR%

where cl >nul 2>&1
if %ERRORLEVEL% neq 0 (
    echo ERROR: MSVC compiler (cl.exe) not found in PATH.
    echo Please run this script from a Visual Studio Developer Command Prompt.
    echo.
    echo For Visual Studio 2022:
    echo   "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars32.bat"
    echo For Visual Studio 2019:
    echo   "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvars32.bat"
    exit /b 1
)

echo Compiling %DLL_NAME% (32-bit)...

cl /nologo /c /O2 /W3 /DWIN32 /D_WINDOWS /D_USRDLL /DNDEBUG /D_CRT_SECURE_NO_WARNINGS ^
    /I%SRC_DIR% ^
    %SRC_DIR%\ev_rclone.c ^
    %SRC_DIR%\ev_http_client.c ^
    %SRC_DIR%\ev_json.c ^
    %SRC_DIR%\ev_rclone_rc.c ^
    %SRC_DIR%\ev_rclone_config.c ^
    %SRC_DIR%\ev_string_util.c

if %ERRORLEVEL% neq 0 (
    echo ERROR: Compilation failed.
    exit /b 1
)

echo Linking %DLL_NAME%...

link /nologo /DLL /OUT:%OUT_DIR%\%DLL_NAME% /DEF:%SRC_DIR%\ev_rclone.def ^
    ev_rclone.obj ev_http_client.obj ev_json.obj ev_rclone_rc.obj ev_rclone_config.obj ev_string_util.obj ^
    ws2_32.lib

if %ERRORLEVEL% neq 0 (
    echo ERROR: Linking failed.
    exit /b 1
)

del /q *.obj 2>nul

echo.
echo Build succeeded: %OUT_DIR%\%DLL_NAME%
echo.

endlocal
