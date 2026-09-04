#ifndef LIVE_VIEW_HANDLER_H
#define LIVE_VIEW_HANDLER_H

#include "App_Globals.h"
#include <TJpg_Decoder.h>
#include <esp_now.h>

void drawLiveHUD() {
    uint16_t hudColor = 0x07FF; // Cyan
    int pad = 10, len = 20;
    // Corner Brackets
    gfx.drawFastHLine(pad, pad, len, hudColor); gfx.drawFastVLine(pad, pad, len, hudColor);
    gfx.drawFastHLine(gfx.width() - pad - len, pad, len, hudColor); gfx.drawFastVLine(gfx.width() - pad, pad, len, hudColor);
    gfx.drawFastHLine(pad, gfx.height() - pad, len, hudColor); gfx.drawFastVLine(pad, gfx.height() - pad - len, len, hudColor);
    gfx.drawFastHLine(gfx.width() - pad - len, gfx.height() - pad, len, hudColor); gfx.drawFastVLine(gfx.width() - pad, gfx.height() - pad - len, len, hudColor);

    gfx.setFont(&fonts::DejaVu12);
    gfx.setTextColor(hudColor);
    gfx.setTextDatum(TR_DATUM);
    char rssiBuf[32];
    snprintf(rssiBuf, sizeof(rssiBuf), "LINK: %d dBm // LIVE", (int)lastRSSI);
    gfx.drawString(rssiBuf, gfx.width() - 15, 15);

    // Back Button Overlay
    gfx.setTextDatum(BC_DATUM);
    gfx.setTextColor(TFT_BLACK, 0x07FF);
    gfx.drawString(" [ BACK TO MENU ] ", gfx.width() / 2, gfx.height() - 5);
}

void processLiveStream() {
    if (frameReady) {
        TJpgDec.setJpgScale(1);
        TJpgDec.drawJpg(0, 0, frameBuffer, currentFramePos);
        drawLiveHUD();
        frameReady = false;
        uint8_t msg = 0x02; // Request next frame
        esp_now_send(broadcastAddress, &msg, 1);
    }
}

#endif