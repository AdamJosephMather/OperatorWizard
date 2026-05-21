@echo off
operatorwizard.exe

if not exist "%TEMP%\temp_cd_path.txt" goto :EOF

set /p NEW_DIR=<"%TEMP%\temp_cd_path.txt"
del "%TEMP%\temp_cd_path.txt"
cd /d "%NEW_DIR%"

:EOF