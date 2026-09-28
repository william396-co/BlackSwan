@echo off
setlocal

rem Resolve paths from this script's directory.
set "SRC_DIR=%~dp0inner"
set "DST_DIR=%~dp0..\share\proto"
set "PROTOC=%~dp0..\..\thirdparty\protobuf-36.1\win-x64\bin\release\protoc.exe"
set "FILE_NAME=commdef.proto"
set "FILE_NAME1=gg_ls.proto"

if not exist "%DST_DIR%" mkdir "%DST_DIR%"

"%PROTOC%" -I="%SRC_DIR%" --cpp_out="%DST_DIR%" "%FILE_NAME%"
"%PROTOC%" -I="%SRC_DIR%" --cpp_out="%DST_DIR%" "%FILE_NAME1%"

echo Generated %FILE_NAME% and %FILE_NAME1%.
endlocal
pause
