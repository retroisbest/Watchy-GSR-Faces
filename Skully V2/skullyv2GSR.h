#ifndef SKULLY_GSR_H
#define SKULLY_GSR_H

#include "Watchy_GSR.h"
#include "pirate.h" // Make sure this defines 'pirate[] PROGMEM'

// Include the font files
#include "fonts/GorgeousPixel30pt7b.h"
#include "fonts/GorgeousPixel11pt7b.h"
#include "fonts/GorgeousPixel9pt7b.h"
#include "fonts/GorgeousPixel8pt7b.h"

#define DISPLAY_WIDTH 200
#define DISPLAY_HEIGHT 200

RTC_DATA_ATTR uint8_t skullyTimeStyle;
int skullyLastMinute = -1;

// Time zone offset in hours (e.g., 1 for UTC+1, -5 for EST)
const int SkullyTimeZone = 1;

class SkullyGSRClass : public WatchyGSR {
public:
    SkullyGSRClass() : WatchyGSR() {
        initAddOn(this);
    }

    void RegisterWatchFaces() {
        skullyTimeStyle = AddWatchStyle("Skully V2", this);
    }

    void InsertInitWatchStyle(uint8_t StyleID) {
        if (StyleID == skullyTimeStyle) {
            Design.Face.Bitmap = nullptr;
            Design.Face.SleepBitmap = nullptr;
            Design.Status.Inverted = false;
        }
    }

    void InsertDrawWatchStyle(uint8_t StyleID) {
        if (StyleID != skullyTimeStyle || !SafeToDraw()) return;

        tmElements_t time;
        SRTC.read(time);
        adjustTimeByOffset(time, SkullyTimeZone);

        display.fillScreen(GxEPD_BLACK);
        display.drawBitmap(0, 0, pirate, 200, 200, GxEPD_WHITE); // Background

        drawTime(time);
        drawWDay(time);
        drawDate(time);
        drawSteps(); // Updated to use CurrentStepCount()
        drawBattery(); // You might need to implement this function or remove it
        

        for (uint8_t i = 0; i < 3; i++) {
            // Reduce ghosting
            display.display(true);
        }
    }

private:
    void adjustTimeByOffset(tmElements_t &tm, int offset) {
        time_t raw = makeTime(tm);
        raw += offset * 3600;  // Adjust time by the offset in seconds
        breakTime(raw, tm);
    }

    void drawTime(const tmElements_t &time) {
        display.setFont(&GorgeousPixel30pt7b); // Use the correct font
        display.setTextColor(GxEPD_WHITE);

        String h = String(time.Hour);
        String m = String(time.Minute);
        String h1 = String(h.substring(0, 1));
        String h2 = String(h.substring(1, 2));
        String m1 = String(m.substring(0, 1));
        String m2 = String(m.substring(1, 2));

        int16_t xh1, yh1;
        uint16_t wh1, hh1;
        int16_t xh2, yh2;
        uint16_t wh2, hh2;
        int16_t xm1, ym1;
        uint16_t wm1, hm1;
        int16_t xm2, ym2;
        uint16_t wm2, hm2;

        display.getTextBounds(String(h1), 0, 0, &xh1, &yh1, &wh1, &hh1);
        display.getTextBounds(String(h2), 0, 0, &xh2, &yh2, &wh2, &hh2);
        display.getTextBounds(String(m1), 0, 0, &xm1, &ym1, &wm1, &hm1);
        display.getTextBounds(String(m2), 0, 0, &xm2, &ym2, &wm2, &hm2);
        display.setCursor(142 - wh1 / 2, 144);

        if (time.Hour < 10) {
            display.print("0");
            display.setCursor(174 - wh1 / 2, 144);
            display.print(String(h1));
        } else {
            display.print(String(h1));
            display.setCursor(174 - wh2 / 2, 144);
            display.print(String(h2));
        }

        display.setCursor(142 - wm1 / 2, 192);

        if (time.Minute < 10) {
            display.print("0");
            display.setCursor(174 - wm1 / 2, 192);
            display.print(String(m1));
        } else {
            display.print(String(m1));
            display.setCursor(174 - wm2 / 2, 192);
            display.print(String(m2));
        }
    }



    void drawDate(const tmElements_t &time) {
        display.setFont(&GorgeousPixel9pt7b); // Use the correct font
        display.setTextColor(GxEPD_WHITE);
        display.setCursor(4, 195);

        String monthStr = String(time.Month + 1);
        String dayStr = String(time.Day);
        monthStr = time.Month < 10 ? "0" + monthStr : monthStr;
        dayStr = time.Day < 10 ? "0" + dayStr : dayStr;
        String dateStr = dayStr + "." + monthStr;
        display.print(String(dateStr));
    }

void drawWDay(const tmElements_t &time) {
    // Adjust for 1-based month
    int displayMonth = time.Month + 1;  // Adjust so January is 1, February is 2, etc.

    // Create a time_t object to correctly calculate the weekday
    tmElements_t adjustedTime = time;
    adjustedTime.Month = displayMonth;  // Adjust month to 1-based

    time_t epoch = makeTime(adjustedTime); // Convert adjusted time to time_t

    // Calculate the weekday index (0 = Sunday, 6 = Saturday)
    uint8_t weekday_index = weekday(epoch) - 1;  // Adjust to 0-based index

    // Define the weekday names
    const char *weekdays[] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};

    // Get the current weekday name
    const char *current_weekday = weekdays[weekday_index];

    // Set font and color for display
    display.setFont(&GorgeousPixel11pt7b); // Use the correct font
    display.setTextColor(GxEPD_BLACK);

    // Calculate text dimensions
    int16_t x1, y1;
    uint16_t w, h;
    String dayOfWeek = String(current_weekday);
    display.getTextBounds(dayOfWeek, 0, 0, &x1, &y1, &w, &h);

    // Clear previous weekday area
    display.fillRect(186 - w, 30, w + 8, h + 5, GxEPD_WHITE);

    // Set cursor position and print the weekday
    display.setCursor(189 - w, 48);
    display.println(dayOfWeek);
}


void drawSteps() {
    display.setFont(&GorgeousPixel9pt7b); // Use the correct font
    display.setTextColor(GxEPD_WHITE);

    // ---------- Today's Steps ----------
    uint32_t stepCount = CurrentStepCount();
    char stepStr[32];
    itoa(stepCount, stepStr, 10);
    int stepStrL = strlen(stepStr);
    memset(stepStr, '0', 5);
    itoa(stepCount, stepStr + max(5 - stepStrL, 0), 10);

    int16_t x1, y1;
    uint16_t w, h;
    display.getTextBounds(String(stepStr), 0, 0, &x1, &y1, &w, &h);
    display.drawRect(186 - w, 51, w + 8, h + 8, GxEPD_WHITE);
    display.setCursor(189 - w, 67);
    display.println(stepStr);

    // ---------- Yesterday's Steps ----------
    uint32_t ystepCount = YesterdayStepCount(); // Or use YesterdaySteps() for formatted
    char ystepStr[32];
    itoa(ystepCount, ystepStr, 10);
    int ystepStrL = strlen(ystepStr);
    memset(ystepStr, '0', 5);
    itoa(ystepCount, ystepStr + max(5 - ystepStrL, 0), 10);

    int16_t yx1, yy1;
    uint16_t yw, yh;
    display.getTextBounds(String(ystepStr), 0, 0, &yx1, &yy1, &yw, &yh);
    display.drawRect(186 - yw, 72, yw + 8, yh + 8, GxEPD_WHITE); // Lower box
    display.setCursor(189 - yw, 88);
    display.println(ystepStr);
}


void drawBattery() {
    float BATTV = getBatteryVoltage() - 3.60;
    int batt_w = constrain(((33.33 * BATTV) + 0.9), 0, 20);

    // Clear previous battery area
    display.fillRect(166, 5, 28, 12, GxEPD_WHITE);

    // Draw the battery outline
    display.fillRect(163, 9, 3, 4, GxEPD_WHITE); // Left side
    display.fillRect(168, 7, 24, 8, GxEPD_BLACK); // Main area

    // Fill the battery level
    if (BATTV > 0) {
        display.fillRect(190 - batt_w, 9, batt_w, 4, GxEPD_WHITE);
    }
}

};

SkullyGSRClass skullyGSRClassLoader;

#endif // SKULLY_GSR_H

