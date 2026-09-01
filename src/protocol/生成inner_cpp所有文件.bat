@echo off
setlocal

rem Resolve paths from this script's directory.
set "SRC_DIR=%~dp0inner"
set "DST_DIR=%~dp0..\share\proto"
set "PROTOC=%~dp0..\..\thirdparty\protobuf-36.1\win-x64\bin\release\protoc.exe"

if not exist "%DST_DIR%" mkdir "%DST_DIR%"

for /f "delims=" %%i in ('dir /b "%SRC_DIR%\*.proto"') do (
    echo "%PROTOC%" -I="%SRC_DIR%" --cpp_out="%DST_DIR%" "%%i"
    "%PROTOC%" -I="%SRC_DIR%" --cpp_out="%DST_DIR%" "%%i"
)

echo Protocol generation complete.
endlocal
pause
