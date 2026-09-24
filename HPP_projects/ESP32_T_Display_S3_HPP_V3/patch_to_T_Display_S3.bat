@echo off
setlocal
title Patch TFT_eSPI for LilyGO T-Display-S3

echo ===================================================================
echo  TFT_eSPI Library Configuration Switcher: LilyGO T-Display-S3
echo ===================================================================
echo.

set TARGET_DIR=%USERPROFILE%\Documents\Arduino\libraries\TFT_eSPI
if not exist "%TARGET_DIR%" (
    set TARGET_DIR=%USERPROFILE%\OneDrive\Documents\Arduino\libraries\TFT_eSPI
)

if not exist "%TARGET_DIR%\User_Setup_Select.h" goto :notfound

copy /Y "%~dp0User_Setup_Select_T_Display_S3.h" "%TARGET_DIR%\User_Setup_Select.h" >nul
if errorlevel 1 goto :fail

echo [SUCCESS] TFT_eSPI configured for LilyGO T-Display-S3!
echo           Setup   : Setup206_LilyGo_T_Display_S3.h
echo           Display : 1.9-inch ST7789 8-bit Parallel (170x320)
echo           Pins    : D0-D7 [39-48], CS 6, DC 7, WR 8, RD 9, RST 5, BL 38, PWR 15
echo.
echo Target file updated:
echo %TARGET_DIR%\User_Setup_Select.h
goto :done

:notfound
echo [ERROR] Could not locate User_Setup_Select.h in:
echo         %TARGET_DIR%
echo Please verify that TFT_eSPI is installed in your Arduino libraries folder.
goto :done

:fail
echo.
echo [ERROR] Failed to copy configuration file.

:done
echo.
pause
