#include "Cloud.h"
#include "Storage.h"
#include "secrets.h"
#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>

#define TIMEOUT_MS 10000

// A fresh client is created per request via httpBegin

// ─── Helpers ─────────────────────────────────────────────────────────────────

static void setHeaders(HTTPClient& http) {
    http.addHeader("Content-Type", "application/json");
    http.addHeader("apikey", SUPABASE_KEY);
    http.addHeader("Authorization", String("Bearer ") + SUPABASE_KEY);
}

static String buildUrl(const char* path) {
    return String(SUPABASE_URL) + "/rest/v1/" + path;
}

static WiFiClientSecure* httpBeginClient = nullptr;

static bool httpBegin(HTTPClient& http, const String& url) {
    if (httpBeginClient) {
        httpBeginClient->stop();
        delete httpBeginClient;
    }
    httpBeginClient = new WiFiClientSecure();
    httpBeginClient->setInsecure();
    httpBeginClient->setTimeout(15);
    return http.begin(*httpBeginClient, url);
}

// Returns the UUID of the device row, or "" if not found
static String getDeviceUUID(const char* deviceId) {
    HTTPClient http;
    http.setTimeout(TIMEOUT_MS);
    httpBegin(http,buildUrl("devices?device_id=eq.") + deviceId + "&select=id");
    setHeaders(http);

    int code = http.GET();
    if (code != 200) { http.end(); return ""; }

    JsonDocument doc;
    deserializeJson(doc, http.getString());
    http.end();

    if (doc.size() == 0) return "";
    return doc[0]["id"].as<String>();
}

// ─── Device config (thresholds) — stored in RTC memory ───────────────────────
// Defined here, declared extern in main.cpp so both can access them
RTC_DATA_ATTR float cachedTargetTemp      = 0;
RTC_DATA_ATTR float cachedThresholdTemp   = 0;
RTC_DATA_ATTR float cachedTargetHum       = 0;
RTC_DATA_ATTR float cachedThresholdHum    = 0;
RTC_DATA_ATTR bool  cachedConfigValid     = false;

bool Cloud::fetchDeviceConfig(const char* deviceId) {
    String url = buildUrl("devices?device_id=eq.") + deviceId +
                 "&select=target_temp,threshold_temp,target_humidity,threshold_humidity";
    HTTPClient http;
    http.setTimeout(TIMEOUT_MS);
    httpBegin(http,url);
    setHeaders(http);

    int code = http.GET();
    if (code != 200) { http.end(); return false; }

    JsonDocument doc;
    deserializeJson(doc, http.getString());
    http.end();

    if (doc.size() == 0 || doc[0]["target_temp"].isNull()) return false;

    cachedTargetTemp    = doc[0]["target_temp"].as<float>();
    cachedThresholdTemp = doc[0]["threshold_temp"].as<float>();
    cachedTargetHum     = doc[0]["target_humidity"].as<float>();
    cachedThresholdHum  = doc[0]["threshold_humidity"].as<float>();
    cachedConfigValid   = true;

    Serial.printf("[Cloud] Config: temp=%.1f±%.1f  hum=%.1f±%.1f\n",
                  cachedTargetTemp, cachedThresholdTemp,
                  cachedTargetHum,  cachedThresholdHum);
    return true;
}

// ─── Device exists check ──────────────────────────────────────────────────────

bool Cloud::deviceExists(const char* deviceId) {
    String uuid = getDeviceUUID(deviceId);
    if (uuid != "") {
        Serial.println("[Cloud] Device found in Supabase.");
        return true;
    }
    Serial.println("[Cloud] Device NOT found in Supabase.");
    return false;
}

// ─── Push buffered readings ───────────────────────────────────────────────────

bool Cloud::pushReadings(const char* deviceId, SensorReading* readings, int count) {
    String uuid = getDeviceUUID(deviceId);
    if (uuid == "") {
        Serial.println("[Cloud] Device UUID not found.");
        return false;
    }

    // Build JSON array of readings
    JsonDocument body;
    JsonArray arr = body.to<JsonArray>();

    for (int i = 0; i < count; i++) {
        JsonObject obj = arr.add<JsonObject>();
        obj["device_id"]     = uuid;
        obj["temperature"]   = readings[i].temperature;
        obj["humidity"]      = readings[i].humidity;
        obj["battery_level"] = readings[i].battery;
        if (strlen(readings[i].timestamp) > 0) {
            obj["read_at"] = readings[i].timestamp;
        }
    }

    String bodyStr;
    serializeJson(body, bodyStr);
    Serial.printf("[Cloud] Sending body: %s\n", bodyStr.c_str());

    HTTPClient http;
    http.setTimeout(TIMEOUT_MS);
    httpBegin(http,buildUrl("sensor_readings"));
    setHeaders(http);
    http.addHeader("Prefer", "return=minimal");

    int code = http.POST(bodyStr);

    if (code == 201) {
        http.end();
        Serial.printf("[Cloud] Pushed %d readings.\n", count);
        return true;
    }

    Serial.printf("[Cloud] Push failed: %d — %s\n", code, http.getString().c_str());
    http.end();
    return false;
}

// ─── Push notification ────────────────────────────────────────────────────────

bool Cloud::pushNotification(const char* deviceId, const char* type, const char* message) {
    // Get device UUID and user_id
    HTTPClient http;
    http.setTimeout(TIMEOUT_MS);
    httpBegin(http,buildUrl("devices?device_id=eq.") + deviceId + "&select=id,user_id");
    setHeaders(http);

    int code = http.GET();
    if (code != 200) { http.end(); return false; }

    JsonDocument doc;
    deserializeJson(doc, http.getString());
    http.end();

    if (doc.size() == 0 || doc[0]["user_id"].isNull()) {
        Serial.println("[Cloud] Cannot push notification — no user_id.");
        return false;
    }

    String deviceUUID = doc[0]["id"].as<String>();
    String userUUID   = doc[0]["user_id"].as<String>();

    // Insert notification
    HTTPClient nhttp;
    nhttp.setTimeout(TIMEOUT_MS);
    httpBegin(nhttp,buildUrl("notifications"));
    setHeaders(nhttp);
    nhttp.addHeader("Prefer", "return=minimal");

    JsonDocument body;
    body["user_id"]   = userUUID;
    body["device_id"] = deviceUUID;
    body["type"]      = type;
    body["message"]   = message;

    String bodyStr;
    serializeJson(body, bodyStr);

    code = nhttp.POST(bodyStr);
    nhttp.end();

    if (code == 201) {
        Serial.printf("[Cloud] Notification sent: %s\n", message);
        return true;
    }

    Serial.printf("[Cloud] Notification failed: %d\n", code);
    return false;
}

// ─── Update device status ─────────────────────────────────────────────────────

bool Cloud::updateDeviceStatus(const char* deviceId) {
    HTTPClient http;
    http.setTimeout(TIMEOUT_MS);
    httpBegin(http,buildUrl("devices?device_id=eq.") + deviceId);
    setHeaders(http);
    http.addHeader("Prefer", "return=minimal");

    JsonDocument body;
    body["connected_at"] = "now()";

    String bodyStr;
    serializeJson(body, bodyStr);

    int code = http.PATCH(bodyStr);
    http.end();

    if (code == 204) {
        Serial.println("[Cloud] Device status updated.");
        return true;
    }

    Serial.printf("[Cloud] Status update failed: %d\n", code);
    return false;
}

// ─── AI insight via Edge Function ────────────────────────────────────────────

bool Cloud::fetchAiInsight(const char* deviceId, float temp, float hum, int battery, char* outBuf, size_t bufLen) {
    String url = String(SUPABASE_URL) + "/functions/v1/analyze-reading";

    HTTPClient http;
    http.setTimeout(TIMEOUT_MS);
    httpBegin(http, url);
    http.addHeader("Content-Type", "application/json");
    http.addHeader("Authorization", String("Bearer ") + SUPABASE_KEY);

    JsonDocument body;
    body["device_id"]   = deviceId;
    body["temperature"] = temp;
    body["humidity"]    = hum;
    body["battery"]     = battery;
    if (cachedConfigValid) {
        body["target_temp"]    = cachedTargetTemp;
        body["threshold_temp"] = cachedThresholdTemp;
        body["target_hum"]     = cachedTargetHum;
        body["threshold_hum"]  = cachedThresholdHum;
    }

    String bodyStr;
    serializeJson(body, bodyStr);

    int code = http.POST(bodyStr);
    if (code != 200) {
        Serial.printf("[Cloud] AI insight failed: %d\n", code);
        http.end();
        return false;
    }

    JsonDocument doc;
    deserializeJson(doc, http.getString());
    http.end();

    const char* insight = doc["insight"] | "";
    strncpy(outBuf, insight, bufLen - 1);
    outBuf[bufLen - 1] = '\0';

    Serial.printf("[Cloud] AI insight: %s\n", outBuf);
    return true;
}

// ─── Fetch pending WiFi networks ──────────────────────────────────────────────

void Cloud::fetchAndSavePendingNetworks(const char* deviceId) {
    String uuid = getDeviceUUID(deviceId);
    if (uuid == "") return;

    HTTPClient http;
    http.setTimeout(TIMEOUT_MS);
    httpBegin(http,buildUrl("device_wifi_networks?device_id=eq.") + uuid + "&state=eq.pending&select=id,ssid,password");
    setHeaders(http);

    int code = http.GET();
    if (code != 200) {
        Serial.printf("[Cloud] Fetch networks failed: %d\n", code);
        http.end();
        return;
    }

    JsonDocument netDoc;
    deserializeJson(netDoc, http.getString());
    http.end();

    int count = netDoc.size();
    Serial.printf("[Cloud] Pending networks: %d\n", count);

    for (int i = 0; i < count; i++) {
        const char* rowId = netDoc[i]["id"];
        const char* ssid  = netDoc[i]["ssid"];
        const char* pass  = netDoc[i]["password"];

        WiFi.begin(ssid, pass);
        unsigned long start = millis();
        while (WiFi.status() != WL_CONNECTED && millis() - start < 10000) {
            delay(200);
        }

        const char* newState;
        if (WiFi.status() == WL_CONNECTED) {
            newState = "connected";
            int idx = Storage::getNetworkCount();
            if (idx < MAX_NETWORKS) Storage::saveNetwork(idx, ssid, pass, "connected");
            WiFi.disconnect();
            Serial.printf("[Cloud] Network saved: %s\n", ssid);
        } else if (WiFi.status() == WL_CONNECT_FAILED) {
            newState = "wrong_password";
        } else {
            newState = "not_available";
        }

        Serial.printf("[Cloud] Network %s → %s\n", ssid, newState);

        // Update state in Supabase
        HTTPClient phttp;
        phttp.setTimeout(TIMEOUT_MS);
        httpBegin(phttp,buildUrl("device_wifi_networks?id=eq.") + rowId);
        setHeaders(phttp);
        phttp.addHeader("Prefer", "return=minimal");

        JsonDocument patchBody;
        patchBody["state"] = newState;
        String patchStr;
        serializeJson(patchBody, patchStr);

        phttp.PATCH(patchStr);
        phttp.end();
    }
}
