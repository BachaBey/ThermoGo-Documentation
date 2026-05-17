#include "Sensor.h"
#include "Battery.h"
#include <Adafruit_BME280.h>

static Adafruit_BME280 bme;

bool Sensor::init() {
    delay(10); // allow BME280 time to stabilise after wake
    if (!bme.begin(0x76)) {
        delay(10);
        if (!bme.begin(0x77)) {
            Serial.println("[Sensor] BME280 not found!");
            return false;
        }
    }
    return true;
}

bool Sensor::read(SensorReading& result) {
    result.temperature = bme.readTemperature();
    result.humidity    = bme.readHumidity();
    result.battery     = Battery::getPercentage();
    result.timestamp[0] = '\0'; // filled by caller from TimeManager

    if (isnan(result.temperature) || isnan(result.humidity)) {
        Serial.println("[Sensor] Read failed");
        return false;
    }

    Serial.printf("[Sensor] Temp=%.1f  Hum=%.1f  Bat=%d%%\n",
                  result.temperature, result.humidity, result.battery);
    return true;
}
