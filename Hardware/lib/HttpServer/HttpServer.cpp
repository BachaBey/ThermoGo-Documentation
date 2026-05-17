#include "HttpServer.h"
#include "Storage.h"
#include <WebServer.h>
#include <ArduinoJson.h>

static WebServer server(80);

static void sendJSON(int code, const String& json) {
    server.send(code, "application/json", json);
}

static void sendError(const char* msg) {
    String j = "{\"status\":\"error\",\"message\":\"";
    j += msg;
    j += "\"}";
    sendJSON(500, j);
}

// ─── GET / ────────────────────────────────────────────────────────────────────
static void handleRoot() {
    JsonDocument doc;
    doc["device_id"] = g_deviceId;
    doc["firmware"]  = g_firmware;
    String out;
    serializeJson(doc, out);
    sendJSON(200, out);
}

// ─── POST /add_wifi ───────────────────────────────────────────────────────────
static void handleAddWifi() {
    if (!server.hasArg("plain")) { sendError("no body"); return; }

    JsonDocument doc;
    if (deserializeJson(doc, server.arg("plain"))) { sendError("bad json"); return; }

    const char* ssid = doc["ssid"]     | "";
    const char* pass = doc["password"] | "";

    if (strlen(ssid) == 0)                          { sendError("missing ssid");     return; }
    if (Storage::getNetworkCount() >= MAX_NETWORKS) { sendError("network list full"); return; }

    Storage::saveNetwork(Storage::getNetworkCount(), ssid, pass, "pending");
    sendJSON(200, "{\"status\":\"ok\"}");
    delay(200);
    ESP.restart();
}

// ─── Route registration ───────────────────────────────────────────────────────

void HttpServer::initConfigMode() {
    server.on("/",         HTTP_GET,  handleRoot);
    server.on("/add_wifi", HTTP_POST, handleAddWifi);
    server.begin();
}

void HttpServer::handleClient() {
    server.handleClient();
}

void HttpServer::stop() {
    server.stop();
}
