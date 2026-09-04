#ifndef CAMERA_CONFIG_HANDLER_H
#define CAMERA_CONFIG_HANDLER_H

#include "App_Globals.h"

void drawCameraConfigScreen() {
    gfx.fillScreen(TFT_BLACK);
    
    // Subtle High-Tech Grid
    for(int i=0; i<=gfx.width(); i+=40) gfx.drawFastVLine(i, 0, gfx.height(), 0x10A2);
    for(int i=0; i<=gfx.height(); i+=40) gfx.drawFastHLine(0, i, gfx.width(), 0x10A2);

    gfx.setFont(&fonts::FreeSansBold9pt7b);
    gfx.setTextColor(0x07FF); // Surgical Cyan
    gfx.setTextDatum(TC_DATUM);
    gfx.drawString("CAMERA CONFIG", gfx.width()/2, 20);

    gfx.setFont(&fonts::DejaVu18);
    gfx.setTextColor(TFT_WHITE);
    gfx.setTextDatum(MC_DATUM);
    gfx.drawString("RES & ZOOM CONTROLS PENDING", gfx.width()/2, gfx.height()/2);

    // Navigation Footer
    gfx.setTextDatum(BC_DATUM);
    gfx.setTextColor(TFT_BLACK, 0x07FF);
    gfx.drawString(" [ TAP TO RETURN ] ", gfx.width()/2, gfx.height() - 20);
}

#endif