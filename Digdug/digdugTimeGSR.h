#ifndef DIGDUGGSR_H
#define DIGDUGGSR_H

#include "Watchy_GSR.h"
#include "digdug.h"

#define DISPLAY_WIDTH 200
#define DISPLAY_HEIGHT 200
#define INDEX_SIZE 4

RTC_DATA_ATTR uint8_t digdugWatchStyle;
const int digdugtimeZone = 1;

const bool digdug_segs[10][7] = {
    {true, true, true, true, true, false, true },   // 0
    {false, false, true, true, false, false, false},// 1
    {false, true, true, false, true, true, true },  // 2
    {false, false, true, true, true, true, true },  // 3
    {true, false, true, true, false, true, false},  // 4
    {true, false, false, true, true, true, true },  // 5
    {true, true, false, true, true, true, true },   // 6
    {false, false, true, true, true, false, false}, // 7
    {true, true, true, true, true, true, true },    // 8
    {true, false, true, true, true, true, true },   // 9
};

extern const uint8_t seg_row[];
extern const uint8_t seg_column[];

const int rock_indexs[8][2] = {
    {3, 7}, {17, 11}, {34, 7}, {43, 10},
    {23, 34}, {4, 41}, {26, 41}, {39, 40}
};

class DigDugGSRClass : public WatchyGSR {
public:
    DigDugGSRClass() : WatchyGSR() {
        initAddOn(this);  // Required
    }

    void RegisterWatchFaces() {
        digdugWatchStyle = AddWatchStyle("Dig Dug", this);
    }

    void InsertInitWatchStyle(uint8_t StyleID) {
        if (StyleID == digdugWatchStyle) {
            Design.Face.Bitmap = nullptr;
            Design.Face.SleepBitmap = nullptr;
            Design.Status.Inverted = false;
        }
    }

    void InsertDrawWatchStyle(uint8_t StyleID) {
        if (StyleID != digdugWatchStyle || !SafeToDraw()) return;

        display.fillScreen(GxEPD_WHITE); // Clear the screen
        drawBackground();   // Draw the background elements
        drawTimeDigits();   // Draw the time digits
        drawDate();         // Draw the date in the top bar
        drawBattery();      // Draw battery level
        drawRock();         // Draw the rocks

        tmElements_t time;
        SRTC.read(time);
        drawEnemiesInsideDigits(time.Hour / 10, 1, 17);  // Enemies inside first hour digit
        drawEnemiesInsideDigits(time.Hour % 10, 12, 17); // Enemies inside second hour digit
        drawEnemiesInsideDigits(time.Minute / 10, 23, 17); // Enemies inside first minute digit
        drawEnemiesInsideDigits(time.Minute % 10, 34, 17); // Enemies inside second minute digit

        display.display(true); // Update the screen
    }

private:
    void adjustTimeByOffset(tmElements_t &t, int offsetHours) {
        time_t rawTime = makeTime(t);
        rawTime += offsetHours * SECS_PER_HOUR;
        breakTime(rawTime, t);
    }

    void drawTimeDigits() {
        tmElements_t time;
        SRTC.read(time);
        adjustTimeByOffset(time, digdugtimeZone);

        draw7Seg(time.Hour / 10, -1, 17);
        draw7Seg(time.Hour % 10, 9, 17);
        draw7Seg(time.Minute / 10, 27, 17);
        draw7Seg(time.Minute % 10, 38, 17);
    }

    void draw7Seg(const int& num, int index_x, int index_y) {
        if (num < 0 || num > 9) return;
        const int scale = 1;

        if (digdug_segs[num][0]) display.drawBitmap(INDEX_SIZE * (index_x + 0), INDEX_SIZE * (index_y + 0), seg_row, INDEX_SIZE * 4 * scale, INDEX_SIZE * 13 * scale, GxEPD_BLACK);
        if (digdug_segs[num][1]) display.drawBitmap(INDEX_SIZE * (index_x + 0), INDEX_SIZE * (index_y + 9 * scale), seg_row, INDEX_SIZE * 4 * scale, INDEX_SIZE * 13 * scale, GxEPD_BLACK);
        if (digdug_segs[num][2]) display.drawBitmap(INDEX_SIZE * (index_x + 6 * scale), INDEX_SIZE * (index_y + 0), seg_row, INDEX_SIZE * 4 * scale, INDEX_SIZE * 13 * scale, GxEPD_BLACK);
        if (digdug_segs[num][3]) display.drawBitmap(INDEX_SIZE * (index_x + 6 * scale), INDEX_SIZE * (index_y + 9 * scale), seg_row, INDEX_SIZE * 4 * scale, INDEX_SIZE * 13 * scale, GxEPD_BLACK);

        if (digdug_segs[num][4]) display.drawBitmap(INDEX_SIZE * (index_x + 0), INDEX_SIZE * (index_y + 0), seg_column, INDEX_SIZE * 10 * scale, INDEX_SIZE * 4 * scale, GxEPD_BLACK);
        if (digdug_segs[num][5]) display.drawBitmap(INDEX_SIZE * (index_x + 0), INDEX_SIZE * (index_y + 9 * scale), seg_column, INDEX_SIZE * 10 * scale, INDEX_SIZE * 4 * scale, GxEPD_BLACK);
        if (digdug_segs[num][6]) display.drawBitmap(INDEX_SIZE * (index_x + 0), INDEX_SIZE * (index_y + 18 * scale), seg_column, INDEX_SIZE * 10 * scale, INDEX_SIZE * 4 * scale, GxEPD_BLACK);
    }

    void drawBackground() {
        display.drawBitmap(0, INDEX_SIZE * 6, ground1, 200, INDEX_SIZE * 10, GxEPD_BLACK);
        display.drawBitmap(0, INDEX_SIZE * 16, ground2, 200, INDEX_SIZE * 10, GxEPD_BLACK);
        display.drawBitmap(0, INDEX_SIZE * 26, ground3, 200, INDEX_SIZE * 10, GxEPD_BLACK);
        display.drawBitmap(0, INDEX_SIZE * 36, ground4, 200, INDEX_SIZE * 10, GxEPD_BLACK);

        display.drawBitmap(INDEX_SIZE * 23, INDEX_SIZE * 6, digdug_center, INDEX_SIZE * 4, INDEX_SIZE * 19, GxEPD_BLACK);
        display.drawBitmap(INDEX_SIZE * 23, INDEX_SIZE * 21, player, INDEX_SIZE * 4, INDEX_SIZE * 4, GxEPD_WHITE);
        display.drawBitmap(INDEX_SIZE * 1, INDEX_SIZE * 46, title, INDEX_SIZE * 12, INDEX_SIZE * 4, GxEPD_BLACK);
    }

    void drawBattery() {
        int8_t batteryLevel = 0;
        float VBAT = getBatteryVoltage();

        if (VBAT > 3.8) batteryLevel = 3;
        else if (VBAT > 3.4) batteryLevel = 2;
        else if (VBAT > 3.0) batteryLevel = 1;

        for (int8_t batterySegments = 0; batterySegments < batteryLevel; batterySegments++) {
            display.drawBitmap(INDEX_SIZE * 46 - INDEX_SIZE * 4 * batterySegments, INDEX_SIZE * 46, player_rest, INDEX_SIZE * 4, INDEX_SIZE * 4, GxEPD_BLACK);
        }
    }

    void drawRock() {
        int choiced[3] = { -1, -1, -1 };
        int randMax = 8;
        for (int cnt = 0; cnt < 3;) {
            int choice = random(randMax);
            bool duplicate = false;
            for (int i = 0; i < cnt; i++) {
                if (choiced[i] == choice) {
                    duplicate = true;
                    break;
                }
            }
            if (!duplicate) choiced[cnt++] = choice;
        }

        for (int i = 0; i < 3; i++) {
            display.drawBitmap(INDEX_SIZE * rock_indexs[choiced[i]][0], INDEX_SIZE * rock_indexs[choiced[i]][1], rock, INDEX_SIZE * 4, INDEX_SIZE * 4, GxEPD_BLACK);
        }
    }

    void drawEnemiesInsideDigits(int digit, int index_x, int index_y) {
        int numEnemies = 3;  // Number of enemies to display inside the digit
        int scale = 1;
        
        // Size of the digits (fixed width)
        int digitWidth = INDEX_SIZE * 4 * scale;
        int digitHeight = INDEX_SIZE * 13 * scale;
        
        for (int i = 0; i < numEnemies; i++) {
            int randX = random(index_x, index_x + digitWidth - 10);  // Padding for enemy size
            int randY = random(index_y, index_y + digitHeight - 10); // Padding for enemy size
            drawEnemy(i, randX, randY, digit + i);  // Draw enemy at random position inside the digit
        }
    }

    void drawEnemy(const int& num, int index_x, int index_y, int seed) {
        randomSeed(seed);
        int choiced = -1;
        while (choiced == -1) {
            int seg = random(7);
            if (digdug_segs[num][seg]) choiced = seg;
        }

        int rand_x = random(7), rand_y = random(10);
        const unsigned char* enemys[] = { waniF, pooka, pookaL, wani, waniL, pooka, pookaL, wani, waniL };
        int rand_enemy = random(9);
        int enemy_size_x = (rand_enemy == 0) ? 10 : 4;
        int enemy_size_y = 4;

        int draw_index_X = index_x, draw_index_Y = index_y;
        switch (choiced) {
            case 0: draw_index_Y += rand_y; break;
            case 1: draw_index_Y += 9 + rand_y; break;
            case 2: draw_index_X += 6; draw_index_Y += rand_y; break;
            case 3: draw_index_X += 6; draw_index_Y += 9 + rand_y; break;
            case 4: draw_index_X += rand_x; break;
            case 5: draw_index_X += rand_x; draw_index_Y += 9; break;
            case 6: draw_index_X += rand_x; draw_index_Y += 18; break;
        }

        display.drawBitmap(INDEX_SIZE * draw_index_X, INDEX_SIZE * draw_index_Y, enemys[rand_enemy], INDEX_SIZE * enemy_size_x, INDEX_SIZE * enemy_size_y, GxEPD_WHITE);
        if (rand_enemy == 0) {
            display.drawBitmap(INDEX_SIZE * draw_index_X, INDEX_SIZE * draw_index_Y, waniFB, INDEX_SIZE * enemy_size_x, INDEX_SIZE * enemy_size_y, GxEPD_BLACK);
        }
    }

void drawDate() {

    tmElements_t currentTime;
    SRTC.read(currentTime);
    int displayMonth = currentTime.Month + 1;  // Adjust so January is 1, February is 2, etc.
    //adjustTimeByOffset(currentTime, digdugtimeZone);

    display.setTextColor(GxEPD_BLACK);
    display.setFont(&FreeMonoBold9pt7b);

    // Format: 1P DDMM
    String score = "1P ";
    if (currentTime.Day < 10) {
        score += "0";
    }
    score += currentTime.Day;
    if (displayMonth < 10) {
        score += "0";
    }
    score += displayMonth;
    display.setCursor(INDEX_SIZE * 0, INDEX_SIZE * 4);
    display.print(score);

    // HIGH score year (4-digit)
    String hiscore = " HIGH ";
    hiscore += tmYearToCalendar(currentTime.Year);
    display.print(hiscore);
}
};

DigDugGSRClass DigDug;
#endif

