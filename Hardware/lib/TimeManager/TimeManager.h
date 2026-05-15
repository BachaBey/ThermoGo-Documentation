#pragma once
#include <Arduino.h>

namespace TimeManager {
    void onBoot(bool timerWake);    // call at the very start of setup()
    void syncFromNTP();             // call after WiFi connects
    String getCurrentTime();        // returns "HH:MM"
    String getTimestamp();          // returns ISO 8601 "2026-03-29T16:08:43+01:00"
    bool isSynced();
}
