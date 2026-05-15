#pragma once
#include <Arduino.h>

// Globals defined in main.cpp, shared with HTTP handlers
extern String g_deviceId;
extern String g_firmware;

namespace HttpServer {
    void initConfigMode();  // GET /, POST /add_wifi
    void handleClient();
    void stop();
}
