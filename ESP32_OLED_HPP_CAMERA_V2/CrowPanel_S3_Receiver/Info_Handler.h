#ifndef INFO_HANDLER_H
#define INFO_HANDLER_H

#include "App_Globals.h"

void drawInfoScreen() {
    gfx.fillScreen(TFT_BLACK);
    for(int i=0; i<=gfx.width(); i+=40) gfx.drawFastVLine(i, 0, gfx.height(), 0x10A2);
    for(int i=0; i<=gfx.height(); i+=40) gfx.drawFastHLine(0, i, gfx.width(), 0x10A2);

    gfx.setFont(&fonts::FreeSansBold9pt7b);
    gfx.setTextColor(0x07FF); // Surgical Cyan
    gfx.setTextDatum(TC_DATUM);
    gfx.drawString("SYSTEM INFO", gfx.width()/2, 20);

    gfx.setFont(&fonts::DejaVu18);
    gfx.setTextColor(TFT_WHITE);
    gfx.setTextDatum(TL_DATUM);
    gfx.setCursor(20, 60);
    gfx.printf("Cam MAC: %02X:%02X:%02X:%02X:%02X:%02X\n", surgicalPenMac[0], surgicalPenMac[1], surgicalPenMac[2], surgicalPenMac[3], surgicalPenMac[4], surgicalPenMac[5]);
    gfx.printf("Signal: %d dBm\n", lastRSSI);
    gfx.printf("Uptime: %lu s\n", millis() / 1000);

    gfx.setTextDatum(BC_DATUM);
    gfx.setTextColor(TFT_BLACK, 0x07FF);
    gfx.drawString(" [ TAP TO RETURN ] ", gfx.width()/2, gfx.height() - 20);
}
#endif