#include "Display.h"
#include <U8g2lib.h>
#include <Wire.h>

static U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE);

void Display::init() {
    u8g2.begin();
    u8g2.clearBuffer();
    u8g2.sendBuffer();
    u8g2.setPowerSave(1); // start with display off
}

void Display::powerOn() {
    u8g2.setPowerSave(0);
}

void Display::powerOff() {
    u8g2.clearBuffer();
    u8g2.sendBuffer();
    u8g2.setPowerSave(1);
}

void Display::showReading(float temperature, float humidity, int battery, const char* timeStr, bool wifiConnected, const char* aiInsight) {
    char tempStr[10];
    char humStr[10];
    char batStr[8];

    snprintf(tempStr, sizeof(tempStr), "%.1f C", temperature);
    snprintf(humStr,  sizeof(humStr),  "%.1f%%", humidity);
    snprintf(batStr,  sizeof(batStr),  "%d%%", battery);

    const char* wifiStr = wifiConnected ? "WiFi" : "----";

    u8g2.clearBuffer();

    // --- Top bar (WiFi uses helvR08, time+battery use 5x7) ---
    u8g2.setFont(u8g2_font_helvR08_tr);
    u8g2.drawStr(0, 9, wifiStr);

    u8g2.setFont(u8g2_font_5x7_tr);

    // Time center
    int timeX = (128 - u8g2.getStrWidth(timeStr)) / 2;
    u8g2.drawStr(timeX, 9, timeStr);

    // Battery percentage
    int batTextWidth = u8g2.getStrWidth(batStr);
    int batTextX = 128 - batTextWidth;
    int iconX = batTextX - 2 - 13;
    u8g2.drawStr(batTextX, 9, batStr);

    // Battery icon body (vertically centered in top bar)
    u8g2.drawFrame(iconX, 2, 11, 7);
    u8g2.drawBox(iconX + 11, 4, 2, 3);
    int fillWidth = (9 * battery) / 100;
    if (fillWidth > 0) {
        u8g2.drawBox(iconX + 1, 3, fillWidth, 5);
    }

    // Divider line
    u8g2.drawHLine(0, 13, 128);

    // --- Values (helvR10, baseline at y=34) ---
    u8g2.setFont(u8g2_font_helvR10_tr);

    int tempValX = (64 - u8g2.getStrWidth(tempStr)) / 2;
    u8g2.drawStr(tempValX, 34, tempStr);

    int humValX = 64 + (64 - u8g2.getStrWidth(humStr)) / 2;
    u8g2.drawStr(humValX, 34, humStr);

    // --- Labels (helvR08, baseline at y=46) ---
    u8g2.setFont(u8g2_font_helvR08_tr);

    int tempLblX = (64 - u8g2.getStrWidth("Temp")) / 2;
    u8g2.drawStr(tempLblX, 44, "Temp");

    int humLblX = 64 + (64 - u8g2.getStrWidth("Hum")) / 2;
    u8g2.drawStr(humLblX, 44, "Hum");

    // --- Bottom line: AI insight if available, otherwise ThermoGo branding ---
    if (aiInsight && strlen(aiInsight) > 0) {
        u8g2.setFont(u8g2_font_helvR08_tr);
        int aiX = (128 - u8g2.getStrWidth(aiInsight)) / 2;
        u8g2.drawStr(aiX, 63, aiInsight);
    } else {
        u8g2.setFont(u8g2_font_helvR10_tr);
        int nameX = (128 - u8g2.getStrWidth("ThermoGo")) / 2;
        u8g2.drawStr(nameX, 63, "ThermoGo");
    }

    u8g2.sendBuffer();
}

void Display::showStatus(const char* message) {
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_helvR08_tr);
    int x = (128 - u8g2.getStrWidth(message)) / 2;
    u8g2.drawStr(x, 32, message);
    u8g2.sendBuffer();
}

void Display::clear() {
    powerOff();
}
