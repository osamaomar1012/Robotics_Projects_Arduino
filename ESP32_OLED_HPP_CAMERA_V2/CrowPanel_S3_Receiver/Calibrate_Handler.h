#ifndef CALIBRATE_HANDLER_H
#define CALIBRATE_HANDLER_H

#include "App_Globals.h"
#include "Menu_Handler.h"

/**
 * @brief Runs the LovyanGFX built-in touch calibration sequence.
 */
void runTouchCalibration() {
    gfx.fillScreen(TFT_BLACK);
    gfx.setFont(&fonts::DejaVu12);
    gfx.setTextColor(0x07FF); // Surgical Cyan
    gfx.setTextDatum(MC_DATUM);
    gfx.drawString("TOUCH CORNER TARGETS", gfx.width() / 2, gfx.height() / 2);
    
    // Built-in calibration: draws targets at corners and waits for input
    uint16_t calData[8];
    gfx.calibrateTouch(calData, 0x07FF, TFT_BLACK, 20);
    
    gfx.fillScreen(TFT_BLACK);
    gfx.drawString("CALIBRATION COMPLETE", gfx.width() / 2, gfx.height() / 2);
    delay(1500);
    currentMode = MODE_MENU;
    drawMenu();
}

#endif