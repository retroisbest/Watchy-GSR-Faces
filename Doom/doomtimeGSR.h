#ifndef DOOMTIMEGSR_H
#define DOOMTIMEGSR_H

#include "Watchy_GSR.h"
#include "doom_gfx.h" // Your image definitions

RTC_DATA_ATTR uint8_t doomTimeStyle;
int doomLastMinute = -1;

// Time zone offset in hours (e.g., 1 for UTC+1, -5 for EST)
const int timeZone = 1;

const unsigned char *doom_imp[2] = {doomImp0, doomImp1};
const unsigned char *doom_idle[2] = {doomImp1, doomImp2};
const unsigned char *doom_impShot[4] = {doomImpshot0, doomImpshot1, doomImpshot2, doomImpshot3};
const unsigned char *doom_gun[4] = {doomgun1, doomgun2, doomgun2, doomgun0};

class DoomTimeGSRClass : public WatchyGSR {
public:
    DoomTimeGSRClass() : WatchyGSR() {
        initAddOn(this);
    }

    void RegisterWatchFaces() {
        doomTimeStyle = AddWatchStyle("Doom Time", this);
    }

    void InsertInitWatchStyle(uint8_t StyleID) {
        if (StyleID == doomTimeStyle) {
            Design.Face.Bitmap = nullptr;
            Design.Face.SleepBitmap = nullptr;
            Design.Status.Inverted = false;
        }
    }

    void InsertDrawWatchStyle(uint8_t StyleID) {
        if (StyleID != doomTimeStyle || !SafeToDraw()) return;

        tmElements_t time;
        SRTC.read(time);

        // Adjust hour for time zone
        int adjustedHour = time.Hour + timeZone;
        if (adjustedHour < 0) {
            adjustedHour += 24;
        } else if (adjustedHour >= 24) {
            adjustedHour -= 24;
        }

        display.fillScreen(GxEPD_WHITE);

        // Draw background elements
        display.drawBitmap(0, 0, doombg1, 50, 59, GxEPD_BLACK);
        display.drawBitmap(138, 0, doombg2, 62, 74, GxEPD_BLACK);
        display.fillRect(0, 147, 200, 53, GxEPD_BLACK);
        display.drawBitmap(0, 145, doomHline, 100, 2, GxEPD_BLACK);
        display.drawBitmap(100, 145, doomHline, 100, 2, GxEPD_BLACK);
        display.drawBitmap(66, 147, doomVline, 2, 53, GxEPD_WHITE);
        display.drawBitmap(132, 147, doomVline, 2, 53, GxEPD_WHITE);

        // Set font and draw time
        display.setTextColor(GxEPD_WHITE);
        display.setFont(&DooM16pt7b);
        char buffer[3];

        // Display adjusted hour
        sprintf(buffer, "%02d", adjustedHour);
        centerJustify(buffer, 31, 178);

        // Display minute
        sprintf(buffer, "%02d", time.Minute);
        centerJustify(buffer, 165, 178);

        // Animate every minute change
        if (time.Minute != doomLastMinute) {
            doomLastMinute = time.Minute;
            display.fillRect(83, 151, 36, 46, GxEPD_BLACK);
            display.drawBitmap(83, 151, doomface1, 36, 46, GxEPD_WHITE);
            for (uint8_t i = 0; i < 4; i++) {
                display.fillRect(56, 93, 95, 52, GxEPD_WHITE);
                display.drawBitmap(56, 93, doom_gun[i % 4], 95, 52, GxEPD_BLACK);
                display.fillRect(50, 0, 88, 93, GxEPD_WHITE);
                display.drawBitmap(50, 0, doom_impShot[i % 4], 88, 93, GxEPD_BLACK);
                display.display(true);
            }
    // ✅ Add these lines to restore idle state
    display.drawBitmap(50, 0, doom_idle[time.Minute % 2], 88, 93, GxEPD_BLACK);
    display.drawBitmap(56, 93, doomgun0, 95, 52, GxEPD_BLACK);
        } else {
            display.drawBitmap(50, 0, doom_idle[time.Minute % 2], 88, 93, GxEPD_BLACK);
            display.drawBitmap(56, 93, doomgun0, 95, 52, GxEPD_BLACK);
        }

        display.fillRect(83, 151, 36, 46, GxEPD_BLACK);
        display.drawBitmap(83, 151, doomface0, 36, 46, GxEPD_WHITE);

        display.display(true);
    }

private:
    void centerJustify(const char *text, int16_t x, int16_t y) {
        int16_t x1, y1;
        uint16_t w, h;
        display.getTextBounds(text, x, y, &x1, &y1, &w, &h);
        display.setCursor(x - (w / 2), y);
        display.print(text);
    }
};

DoomTimeGSRClass doomTimeGSRClassLoader;

#endif // DOOMTIMEGSR_H

