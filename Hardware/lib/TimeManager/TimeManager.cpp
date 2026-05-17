#include "TimeManager.h"
#include <Arduino.h>
#include <time.h>

RTC_DATA_ATTR static bool rtcSynced = false;

void TimeManager::onBoot(bool timerWake) {
    setenv("TZ", "CET-1", 1); // UTC+1 Tunisia
    tzset();
}

void TimeManager::syncFromNTP() {
    configTime(0, 0, "pool.ntp.org", "time.nist.gov"); // offset handled by TZ env var

    struct tm info;
    unsigned long start = millis();
    while (!getLocalTime(&info) && millis() - start < 5000) {
        delay(200);
    }

    if (getLocalTime(&info)) {
        rtcSynced = true;
        Serial.printf("[Time] Synced: %02d:%02d:%02d\n", info.tm_hour, info.tm_min, info.tm_sec);
    } else {
        Serial.println("[Time] NTP sync failed");
    }
}

String TimeManager::getCurrentTime() {
    if (!rtcSynced) return "--:--";

    struct tm info;
    if (!getLocalTime(&info)) return "--:--";

    char buf[6];
    snprintf(buf, sizeof(buf), "%02d:%02d", info.tm_hour, info.tm_min);
    return String(buf);
}

String TimeManager::getTimestamp() {
    if (!rtcSynced) return "";

    struct tm info;
    if (!getLocalTime(&info)) return "";

    char buf[26];
    // Format: 2026-03-29T16:08:43+01:00
    snprintf(buf, sizeof(buf), "%04d-%02d-%02dT%02d:%02d:%02d+01:00",
             info.tm_year + 1900, info.tm_mon + 1, info.tm_mday,
             info.tm_hour, info.tm_min, info.tm_sec);
    return String(buf);
}

bool TimeManager::isSynced() {
    return rtcSynced;
}
