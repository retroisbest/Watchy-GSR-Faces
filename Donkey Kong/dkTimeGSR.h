#ifndef DKTIMEGSR_H
#define DKTIMEGSR_H

#include "Watchy_GSR.h"
#include "dkong.h"

RTC_DATA_ATTR uint8_t dkTimeStyle;
int dkTimeState = 0;

const unsigned char *dk_nums[10] = {dk0, dk1, dk2, dk3, dk4, dk5, dk6, dk7, dk8, dk9};
const unsigned char *dk_anim1[4] = {dk_1, dk_2, dk_1, dk_2};
const unsigned char *dk_anim2[4] = {dk_3, dk_4, dk_3, dk_4};
const unsigned char *dk_anim3[16] = {dk_5, dk_6, dk_7, dk_8, dk_5, dk_6, dk_7, dk_8};

// Variables for tracking the time and triggering animations
int lastMinute = -1; // Initialize to an invalid minute value
int timeZoneOffset = 1; // Adjust this to your time zone (e.g., 1 for UTC+1)

class dkTimeGSRClass : public WatchyGSR {
public:
    dkTimeGSRClass() : WatchyGSR() {
        initAddOn(this);
    }

    void RegisterWatchFaces() {
        dkTimeStyle = AddWatchStyle("DK Time", this);
    }

    void InsertInitWatchStyle(uint8_t StyleID) {
        if (StyleID == dkTimeStyle) {
            Design.Face.Bitmap = nullptr;
            Design.Face.SleepBitmap = nullptr;
            Design.Status.Inverted = false;
        }
    }

    void InsertDrawWatchStyle(uint8_t StyleID) {
        if (StyleID != dkTimeStyle || !SafeToDraw()) return;

        tmElements_t time;
        SRTC.read(time); // Directly read time from the RTC

        int Hour = time.Hour + timeZoneOffset; // Apply time zone offset
        if (Hour >= 24) { // Handle the case where the hour goes beyond 23 (wrap around)
            Hour -= 24;
        } else if (Hour < 0) { // Handle negative hours (wrap around)
            Hour += 24;
        }

        bool isTwelveHour = false;  // Set to false for 24-hour format
        if (isTwelveHour) {
            if (Hour == 0) {
                Hour = 12;
            } else if (Hour > 12) {
                Hour -= 12;
            }
        } else {
            if (Hour == 0) {
                Hour = 12;  // Optional for midnight formatting
            }
        }

        display.fillScreen(GxEPD_BLACK);

        // Drawing the top and bottom background
        display.drawBitmap(0, 0, bg_top, 200, 73, GxEPD_WHITE);
        display.drawBitmap(0, 196, bg_bot, 200, 4, GxEPD_WHITE);

        // Drawing the time digits (hours and minutes)
        display.drawBitmap(35, 33, dk_nums[Hour / 10], 28, 26, GxEPD_WHITE); // Hour tens
        display.drawBitmap(66, 33, dk_nums[Hour % 10], 28, 26, GxEPD_WHITE); // Hour ones
        display.drawBitmap(105, 33, dk_nums[time.Minute / 10], 28, 26, GxEPD_WHITE); // Minute tens
        display.drawBitmap(136, 33, dk_nums[time.Minute % 10], 28, 26, GxEPD_WHITE); // Minute ones

        // Drawing the DK static images
        display.drawBitmap(0, 200, dk_1, 182, 121, GxEPD_WHITE);
        display.drawBitmap(182, 200, dk_2, 182, 121, GxEPD_WHITE);
        display.drawBitmap(0, 321, dk_3, 182, 121, GxEPD_WHITE);
        display.drawBitmap(182, 321, dk_4, 182, 121, GxEPD_WHITE);
        display.drawBitmap(0, 442, dk_5, 200, 121, GxEPD_WHITE);
        display.drawBitmap(200, 442, dk_6, 200, 121, GxEPD_WHITE);
        display.drawBitmap(0, 563, dk_7, 200, 121, GxEPD_WHITE);
        display.drawBitmap(200, 563, dk_8, 200, 121, GxEPD_WHITE);

        // Check if the minute has changed
        if (time.Minute != lastMinute) {
            lastMinute = time.Minute;  // Update the last minute
            //Serial.println("Minute changed, triggering animation!");  // Debugging line
            drawRandomDkAnim();  // Trigger a random animation
        }

        // Update the display
        display.display(true);
    }

    void drawRandomDkAnim() {
        // Choose a random animation
        int randNum = random(0, 3);  // 3 possibilities for animation

        if (randNum == 0) {
            // Animation 1
            for (uint8_t i = 0; i < 4; i++) {
                display.drawBitmap(7, 76, dk_anim1[i % 4], 182, 121, GxEPD_WHITE);
                display.display(true);
                display.fillRect(0, 76, 200, 121, GxEPD_BLACK);  // Clear area before next frame
            }
        } else if (randNum == 1) {
            // Animation 2
            for (uint8_t i = 0; i < 4; i++) {
                display.drawBitmap(7, 76, dk_anim2[i % 4], 182, 121, GxEPD_WHITE);
                display.display(true);
                display.fillRect(0, 76, 200, 121, GxEPD_BLACK);  // Clear area before next frame
            }
        } else {
            // Animation 3
            for (uint8_t i = 0; i < 8; i++) {
                display.drawBitmap(0, 76, dk_anim3[i % 8], 200, 121, GxEPD_WHITE);
                display.display(true);
                display.fillRect(0, 76, 200, 121, GxEPD_BLACK);  // Clear area before next frame
            }
        }

        // Hide ghosting and reset animation area
        display.fillRect(0, 76, 200, 121, GxEPD_BLACK);
        display.drawBitmap(7, 76, dk_anim1[0], 182, 121, GxEPD_WHITE);
        display.display(true);
    }
};

dkTimeGSRClass dkTimeGSRClassLoader;

#endif // DKTIMEGSR_H

