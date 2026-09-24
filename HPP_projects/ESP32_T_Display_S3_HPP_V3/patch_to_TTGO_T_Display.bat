@echo off
setlocal
title Patch TFT_eSPI for TTGO T-Display (ESP32)

echo ===================================================================
echo  TFT_eSPI Library Configuration Switcher: TTGO T-Display (ESP32)
echo ===================================================================
echo.

set TARGET_DIR=%USERPROFILE%\Documents\Arduino\libraries\TFT_eSPI
if not exist "%TARGET_DIR%" (
    set TARGET_DIR=%USERPROFILE%\OneDrive\Documents\Arduino\libraries\TFT_eSPI
)

if not exist "%TARGET_DIR%\User_Setup_Select.h" goto :notfound

copy /Y "%~dp0User_Setup_Select_TTGO_T_Display.h" "%TARGET_DIR%\User_Setup_Select.h" >nul
if errorlevel 1 goto :fail

echo [SUCCESS] TFT_eSPI configured for classic TTGO T-Display (ESP32)!
echo           Setup   : Setup25_TTGO_T_Display.h
echo           Display : 1.14-inch ST7789 SPI (135x240)
echo           Pins    : MOSI 19, SCLK 18, CS 5, DC 16, RST 23, BL 4
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
