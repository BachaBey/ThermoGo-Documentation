#include <Arduino.h>
#include <Wire.h>
#include <WiFi.h>
#include "Display.h"
#include "Motion.h"
#include "Battery.h"
#include "Storage.h"
#include "WiFiManager.h"
#include "Hotspot.h"
#include "HttpServer.h"
#include "TimeManager.h"
#include "Sensor.h"
#include "Cloud.h"

#define SLEEP_MIN_US       60ULL * 1000000    //  1 min  — volatile environment
#define SLEEP_MID_US      300ULL * 1000000    //  5 min  — moderate
#define SLEEP_MAX_US      600ULL * 1000000    // 10 min  — stable environment
#define CLOUD_PUSH_EVERY    10                // push every 10 wakes
#define HOTSPOT_BTN_PIN     0                 // BOOT button — hold 2s to enter hotspot
#define HISTORY_SIZE        10                // rolling window for volatility

// Globals shared with HttpServer
String g_deviceId = "";
String g_firmware = "1.0.0";

// Persists across deep sleep
RTC_DATA_ATTR int           wakeCount         = 0;
RTC_DATA_ATTR int           failCounter       = 0;
RTC_DATA_ATTR bool          firstBoot         = true;
RTC_DATA_ATTR SensorReading readingBuffer[CLOUD_BUFFER_SIZE];
RTC_DATA_ATTR int           bufferCount       = 0;
RTC_DATA_ATTR float         lastTemp          = 0.0;
RTC_DATA_ATTR float         lastHum           = 0.0;
RTC_DATA_ATTR int           sensorFailCount   = 0;
RTC_DATA_ATTR float         tempHistory[HISTORY_SIZE];
RTC_DATA_ATTR int           historyIndex      = 0;
RTC_DATA_ATTR int           historyCount      = 0;
RTC_DATA_ATTR char          aiInsight[24]     = "";

// Threshold cache — defined in Cloud.cpp
extern RTC_DATA_ATTR float cachedTargetTemp;
extern RTC_DATA_ATTR float cachedThresholdTemp;
extern RTC_DATA_ATTR float cachedTargetHum;
extern RTC_DATA_ATTR float cachedThresholdHum;
extern RTC_DATA_ATTR bool  cachedConfigValid;

static float calculateVolatility() {
    if (historyCount < 2) return 0.0f;
    int n = min(historyCount, HISTORY_SIZE);
    float mean = 0.0f;
    for (int i = 0; i < n; i++) mean += tempHistory[i];
    mean /= n;
    float variance = 0.0f;
    for (int i = 0; i < n; i++) variance += pow(tempHistory[i] - mean, 2);
    return sqrt(variance / n);
}

static uint64_t adaptiveSleepInterval() {
    if (historyCount < 2) {
        Serial.println("[Sleep] No history yet → 1 min");
        return SLEEP_MIN_US;
    }
    float stddev = calculateVolatility();
    uint64_t interval;
    if      (stddev < 0.3f) interval = SLEEP_MAX_US;
    else if (stddev < 1.0f) interval = SLEEP_MID_US;
    else                    interval = SLEEP_MIN_US;
    Serial.printf("[Sleep] Volatility: %.2f°C stddev → %llu min\n",
                  stddev, interval / 60000000ULL);
    return interval;
}

void goToSleep() {
    Motion::enableWakeOnMotion();
    esp_sleep_enable_timer_wakeup(adaptiveSleepInterval());
    Serial.println("[Main] Going to sleep.");
    esp_deep_sleep_start();
}

bool isOutsideThreshold(float temp, float hum) {
    if (!cachedConfigValid) return false;
    bool tempAlert = abs(temp - cachedTargetTemp) > cachedThresholdTemp;
    bool humAlert  = abs(hum  - cachedTargetHum)  > cachedThresholdHum;
    return tempAlert || humAlert;
}

void connectAndPush(bool forceSync) {
    Serial.printf("[Main] Saved network count: %d\n", Storage::getNetworkCount());
    if (Storage::getNetworkCount() == 0) {
        Display::powerOn();
        Display::showStatus("Setup mode...");
        Hotspot::start();
        return;
    }

    WiFiManager::init();
    bool wifiOk = WiFiManager::connect();

    if (!wifiOk) {
        failCounter++;
        Serial.printf("[Main] WiFi failed. Fail count: %d/10\n", failCounter);
        if (failCounter >= 10) {
            failCounter = 0;
            Display::powerOn();
            Display::showStatus("No WiFi found");
            delay(2000);
            Hotspot::start();
        }
        return;
    }

    failCounter = 0;
    TimeManager::syncFromNTP();

    // Device not in Supabase → factory reset
    if (!Cloud::deviceExists(g_deviceId.c_str())) {
        Serial.println("[Main] Device not registered — factory reset.");
        Storage::clearNetworks();
        bufferCount = 0;
        wakeCount   = 0;
        Display::powerOn();
        Display::showStatus("Not registered");
        delay(2000);
        Hotspot::start();
        return;
    }

    Cloud::updateDeviceStatus(g_deviceId.c_str());

    // ── Step 1: Fetch latest thresholds from Supabase ────────────────────────
    Cloud::fetchDeviceConfig(g_deviceId.c_str());

    // ── Step 2: Sensor failure alert ─────────────────────────────────────────
    if (sensorFailCount >= 10) {
        Cloud::pushNotification(g_deviceId.c_str(), "sensor_error",
                                "Sensor is not responding for the last 5 minutes. Please check your device.");
        sensorFailCount = 0;
    }

    // ── Step 3: Check each buffered reading against fresh thresholds ─────────
    bool thresholdAlert = false;
    for (int i = 0; i < bufferCount; i++) {
        if (isOutsideThreshold(readingBuffer[i].temperature, readingBuffer[i].humidity)) {
            thresholdAlert = true;
            Serial.printf("[Main] Reading %d exceeds threshold.\n", i);
            break;
        }
    }

    // ── Step 4: Push readings ─────────────────────────────────────────────────
    if (thresholdAlert || forceSync) {
        if (bufferCount > 0) {
            Cloud::pushReadings(g_deviceId.c_str(), readingBuffer, bufferCount);
            bufferCount = 0;
        }
    }

    // ── Step 5: Fetch pending WiFi networks on scheduled push only ────────────
    if (forceSync) {
        Cloud::fetchAndSavePendingNetworks(g_deviceId.c_str());
    }
}

void setup() {
    Serial.begin(115200);
    Wire.begin();

    // Disable onboard LED
    pinMode(2, OUTPUT);
    digitalWrite(2, LOW);

    // Disable Bluetooth
    btStop();

    // Lower CPU to 80MHz — sufficient for sensors and HTTP
    setCpuFrequencyMhz(80);

    esp_sleep_wakeup_cause_t wakeReason = esp_sleep_get_wakeup_cause();
    bool wokenByMotion = (wakeReason == ESP_SLEEP_WAKEUP_EXT0);

    TimeManager::onBoot(!wokenByMotion);
    Storage::init();
    Battery::init();
    Display::init();
    Motion::init();

    // I2C SCAN — remove after diagnosing display
    Serial.println("[I2C] Scanning...");
    int found = 0;
    for (uint8_t addr = 1; addr < 127; addr++) {
        Wire.beginTransmission(addr);
        if (Wire.endTransmission() == 0) {
            Serial.printf("[I2C] Device at 0x%02X\n", addr);
            found++;
        }
    }
    if (found == 0) Serial.println("[I2C] No devices found.");

    // SCREEN TEST — remove after confirming display works
    Display::powerOn();
    Display::showStatus("Screen OK");
    delay(2000);
    Display::clear();

    g_deviceId = Storage::getDeviceId();

    Serial.printf("[Main] Wake: %s  wakeCount=%d\n",
                  wokenByMotion ? "TAP" : "TIMER", wakeCount);

    // ── Tap wake → show screen, allow hotspot trigger via BOOT button ────────
    if (wokenByMotion) {
        pinMode(HOTSPOT_BTN_PIN, INPUT_PULLUP);
        String timeStr = TimeManager::getCurrentTime();
        Display::powerOn();
        Display::showReading(lastTemp, lastHum, Battery::getPercentage(),
                             timeStr.c_str(), false, aiInsight);

        unsigned long start = millis();
        while (millis() - start < 5000) {
            if (digitalRead(HOTSPOT_BTN_PIN) == LOW) {
                Display::showStatus("Hold for WiFi...");
                unsigned long holdStart = millis();
                while (digitalRead(HOTSPOT_BTN_PIN) == LOW && millis() - holdStart < 2000) {
                    delay(50);
                }
                if (millis() - holdStart >= 2000) {
                    Display::showStatus("Starting hotspot...");
                    delay(1000);
                    Hotspot::start();
                    return;
                }
                // Short press — restore reading screen
                Display::showReading(lastTemp, lastHum, Battery::getPercentage(),
                                     timeStr.c_str(), false, aiInsight);
            }
            delay(50);
        }

        Display::clear();
        goToSleep();
        return;
    }

    // ── Timer wake ────────────────────────────────────────────────────────────

    // First boot → connect to WiFi immediately before sensor cycle starts
    if (firstBoot) {
        firstBoot = false;
        Serial.println("[Main] First boot — connecting to WiFi.");
        if (Storage::getNetworkCount() == 0) {
            Display::powerOn();
            Display::showStatus("Setup mode...");
            Hotspot::start();
            return;
        }
        WiFiManager::init();
        if (WiFiManager::connect()) {
            failCounter = 0;
            TimeManager::syncFromNTP();

            // If device is not yet in Supabase, wait 90s for the app to register it
            if (!Cloud::deviceExists(g_deviceId.c_str())) {
                Serial.println("[Main] Waiting 90s for app to register device...");
                Display::powerOn();
                Display::showStatus("Registering...");
                delay(90000);
            }

            if (!Cloud::deviceExists(g_deviceId.c_str())) {
                Serial.println("[Main] Device not registered — factory reset.");
                Storage::clearNetworks();
                Display::powerOn();
                Display::showStatus("Not registered");
                delay(2000);
                Hotspot::start();
                return;
            }
            Cloud::updateDeviceStatus(g_deviceId.c_str());
            Cloud::fetchDeviceConfig(g_deviceId.c_str());
            Cloud::fetchAndSavePendingNetworks(g_deviceId.c_str());
        }
        goToSleep();
        return;
    }

    wakeCount++;

    // Read sensor — fall back to last known values if sensor fails
    SensorReading reading;
    bool sensorOk = Sensor::init() && Sensor::read(reading);
    if (!sensorOk) {
        Serial.println("[Sensor] Using last known values.");
        reading.temperature  = lastTemp;
        reading.humidity     = lastHum;
        reading.battery      = Battery::getPercentage();
        reading.timestamp[0] = '\0';
        sensorFailCount++;
        Serial.printf("[Sensor] Consecutive failures: %d/10\n", sensorFailCount);
    } else {
        sensorFailCount = 0;
    }

    lastTemp = reading.temperature;
    lastHum  = reading.humidity;

    // Update rolling history for adaptive sleep
    tempHistory[historyIndex] = reading.temperature;
    historyIndex = (historyIndex + 1) % HISTORY_SIZE;
    if (historyCount < HISTORY_SIZE) historyCount++;
    String ts = TimeManager::getTimestamp();
    strncpy(reading.timestamp, ts.c_str(), sizeof(reading.timestamp) - 1);
    reading.timestamp[sizeof(reading.timestamp) - 1] = '\0';

    if (bufferCount < CLOUD_BUFFER_SIZE) {
        readingBuffer[bufferCount++] = reading;
    }

    // Threshold check on every reading — push immediately if outside interval
    if (isOutsideThreshold(reading.temperature, reading.humidity)) {
        Serial.println("[Main] Threshold exceeded — pushing immediately.");
        connectAndPush(false);
        Cloud::fetchAiInsight(g_deviceId.c_str(), lastTemp, lastHum,
                              Battery::getPercentage(), aiInsight, sizeof(aiInsight));
        goToSleep();
        return;
    }

    // Scheduled push every CLOUD_PUSH_EVERY wakes
    if (wakeCount >= CLOUD_PUSH_EVERY) {
        wakeCount = 0;
        connectAndPush(true);
        Cloud::fetchAiInsight(g_deviceId.c_str(), lastTemp, lastHum,
                              Battery::getPercentage(), aiInsight, sizeof(aiInsight));
    }

    goToSleep();
}

void loop() {}
