#ifndef KEENTIMEGSR_H
#define KEENTIMEGSR_H

#include "Watchy_GSR.h"
#include "keen_graphics.h"

#define DISPLAY_WIDTH 200
#define DISPLAY_HEIGHT 200

RTC_DATA_ATTR uint8_t keenTimeStyle;
int keenLastMinute = -1;

// Time zone offset in hours (e.g., 1 for UTC+1, -5 for EST)
const int KeentimeZone = 1;

class KeenTimeGSRClass : public WatchyGSR {
public:
    KeenTimeGSRClass() : WatchyGSR() {
        initAddOn(this);
    }

    void RegisterWatchFaces() {
        keenTimeStyle = AddWatchStyle("Commander Keen", this);
    }

    void InsertInitWatchStyle(uint8_t StyleID) {
        if (StyleID == keenTimeStyle) {
            Design.Face.Bitmap = nullptr;
            Design.Face.SleepBitmap = nullptr;
            Design.Status.Inverted = false;
        }
    }

    void InsertDrawWatchStyle(uint8_t StyleID) {
        if (StyleID != keenTimeStyle || !SafeToDraw()) return;

        tmElements_t time;
        SRTC.read(time);
        adjustTimeByOffset(time, KeentimeZone);

        // Debugging output to check month and hour
        Serial.begin(115200); // Only needed once for debugging
        Serial.print("Hour: "); Serial.println(time.Hour);
        Serial.print("Minute: "); Serial.println(time.Minute);
        Serial.print("Month: "); Serial.println(time.Month);
        Serial.print("Day: "); Serial.println(time.Day);

        display.fillScreen(GxEPD_BLACK);

        drawBackground(time);
        drawTime(time);
        drawDate(time);
        drawWeekday(time);
        drawBattery();
        display.display(true);
    }

private:
    void adjustTimeByOffset(tmElements_t &tm, int offset) {
        time_t raw = makeTime(tm);
        raw += offset * 3600;  // Adjust time by the offset in seconds
        breakTime(raw, tm);
    }

    void drawBackground(const tmElements_t &time) {
        time_t epoch = makeTime(time);
        uint8_t background_number = epoch / 60 / 10 % BACKGROUNDS;
        const unsigned char *background = backgrounds[background_number];
        display.drawBitmap(0, 0, background, DISPLAY_WIDTH, DISPLAY_HEIGHT, GxEPD_WHITE);
    }

    void drawTime(const tmElements_t &time) {
        display.setFont(&keen_score_font);
        display.setTextColor(GxEPD_WHITE);
        display.setCursor(51, 12);
        display.print(";");

        int hour = time.Hour; // 24-hour format (do not use %12)
        if (hour < 10) display.print(";");
        display.print(hour);
        display.print(":");
        if (time.Minute < 10) display.print("0");
        display.print(time.Minute);
    }

void drawDate(const tmElements_t &time) {
    display.setFont(&keen_score_font);
    display.setTextColor(GxEPD_WHITE);

    // Adjust for 0-indexed month (0 = January, 1 = February, ..., 11 = December)
    int displayMonth = time.Month + 1;  // Adjust so January is 1, February is 2, etc.

    // Month display with leading zero if less than 10
    display.setCursor(35, 44);
    if (displayMonth < 10) display.print("0");  // Add leading zero
    display.print(displayMonth);

    // Day display with leading zero if less than 10
    display.setCursor(115, 44);
    if (time.Day < 10) display.print("0");  // Add leading zero
    display.print(time.Day);
}



void drawWeekday(const tmElements_t &time) {
    // Adjust for 1-based month
    int displayMonth = time.Month + 1;  // Adjust so January is 1, February is 2, etc.
    
    // Create a time_t object to correctly calculate the weekday
    tmElements_t adjustedTime = time;
    adjustedTime.Month = displayMonth;  // Adjust month to 1-based

    time_t epoch = makeTime(adjustedTime); // Convert adjusted time to time_t

    uint8_t weekday_index = weekday(epoch) - 1;  // Weekday function returns 1 for Sunday, 7 for Saturday
    const unsigned char *current_weekday = weekdays[weekday_index];
    
    // Draw the weekday image
    display.drawBitmap(19, 12, current_weekday, WEEKDAY_WIDTH, WEEKDAY_HEIGHT, GxEPD_WHITE);
}

    void drawBattery() {
        float voltage = getBatteryVoltage();
        const float full = 4.1, empty = 3.7;
        const int maxSweep = 13, step = 7, segments = 3;

        float stepV = (full - empty) / (maxSweep + step * (segments - 1));
        int idx = 0;

        for (; idx < maxSweep + step * (segments - 1); idx++) {
            if ((stepV * idx + empty) >= voltage) break;
        }

        uint8_t bars[3] = {
            min(idx, maxSweep),
            max(min(idx - step, maxSweep), 0),
            max(idx - 2 * step, 0)
        };

        for (uint8_t i = 0; i < 3; i++) {
            display.drawBitmap(165 + i * 9, 180, battery_meter_bars[bars[i]], 7, 11, GxEPD_BLACK);
        }
    }
};

KeenTimeGSRClass keenTimeGSRClassLoader;

#endif // KEENTIMEGSR_H

