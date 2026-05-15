#include "Hotspot.h"
#include "Storage.h"
#include "HttpServer.h"
#include "Display.h"
#include <WiFi.h>

#define HOTSPOT_TIMEOUT_MS 300000  // 5 minutes
#define HOTSPOT_BTN_PIN    0       // BOOT button — hold 2s to exit hotspot

void Hotspot::start() {
    String deviceId = Storage::getDeviceId();
    String apName   = "ThermoGo-" + deviceId.substring(deviceId.length() - 4);

    WiFi.mode(WIFI_AP);
    WiFi.softAP(apName.c_str());
    Serial.printf("[Hotspot] AP started: %s  IP: %s\n",
                  apName.c_str(),
                  WiFi.softAPIP().toString().c_str());

    pinMode(HOTSPOT_BTN_PIN, INPUT_PULLUP);
    Display::powerOn();
    Display::showStatus(("WiFi: " + apName).c_str());
    HttpServer::initConfigMode();

    unsigned long start    = millis();
    unsigned long btnStart = 0;
    bool          btnHeld  = false;

    while (millis() - start < HOTSPOT_TIMEOUT_MS) {
        HttpServer::handleClient();

        if (digitalRead(HOTSPOT_BTN_PIN) == LOW) {
            if (!btnHeld) {
                btnHeld  = true;
                btnStart = millis();
                Display::showStatus("Hold to exit...");
            } else if (millis() - btnStart >= 2000) {
                Serial.println("[Hotspot] User exited hotspot.");
                HttpServer::stop();
                WiFi.softAPdisconnect(true);
                Display::showStatus("Exiting hotspot");
                delay(1000);
                Display::clear();
                esp_deep_sleep_start();
                return;
            }
        } else {
            if (btnHeld) {
                btnHeld = false;
                Display::showStatus("Connect to:");
            }
        }

        delay(10);
    }

    // Timeout — no provisioning happened
    HttpServer::stop();
    WiFi.softAPdisconnect(true);
    Serial.println("[Hotspot] Timeout. Going to sleep.");
    esp_deep_sleep(5ULL * 60 * 1000000);
}
