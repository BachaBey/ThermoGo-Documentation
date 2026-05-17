#pragma once

namespace Display {
    void init();
    void powerOn();
    void powerOff();
    void showReading(float temperature, float humidity, int battery, const char* timeStr, bool wifiConnected, const char* aiInsight = "");
    void showStatus(const char* message);
    void clear();
}
