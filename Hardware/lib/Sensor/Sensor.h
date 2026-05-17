#pragma once
#include <Arduino.h>

struct SensorReading {
    float temperature;
    float humidity;
    int   battery;
    char  timestamp[26]; // ISO 8601: "2026-03-29T16:08:43+01:00" + null
};

namespace Sensor {
    bool init();
    bool read(SensorReading& result);
}
