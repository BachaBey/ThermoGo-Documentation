#include "Battery.h"
#include <Arduino.h>
#include <math.h>

#define BAT_PIN        34
#define R1             4700.0f    // 4.7kΩ
#define R2             10000.0f   // 10kΩ
#define BAT_MAX        4.2f
#define BAT_MIN        3.0f
#define NUM_SAMPLES    16

// NTC thermistor parameters
#define NTC_PULLUP     10000.0f   // 10kΩ pull-up resistor
#define NTC_R0         10000.0f   // NTC resistance at 25°C
#define NTC_T0         298.15f    // 25°C in Kelvin
#define NTC_BETA       3950.0f    // Beta coefficient (typical LiPo NTC)

void Battery::init() {
    analogSetPinAttenuation(BAT_PIN, ADC_11db);
    pinMode(BAT_PIN, INPUT);
    analogSetPinAttenuation(BAT_TEMP_PIN, ADC_11db);
    pinMode(BAT_TEMP_PIN, INPUT);
}

float Battery::getVoltage() {
    long sum = 0;
    for (int i = 0; i < NUM_SAMPLES; i++) {
        sum += analogReadMilliVolts(BAT_PIN); // uses ESP32 built-in eFuse calibration
        delay(5);
    }
    float vadc = (sum / (float)NUM_SAMPLES) / 1000.0f; // mV → V
    float vbat = vadc * ((R1 + R2) / R2);              // reverse the divider
    return vbat;
}

int Battery::getPercentage() {
    float vbat = getVoltage();
    float pct = (vbat - BAT_MIN) / (BAT_MAX - BAT_MIN) * 100.0f;
    if (pct > 100.0f) return 100;
    if (pct < 0.0f)   return 0;
    return (int)pct;
}

float Battery::getTemperature() {
    float vmv = analogReadMilliVolts(BAT_TEMP_PIN);
    float vntc = vmv / 1000.0f;
    float rntc = NTC_PULLUP * vntc / (3.3f - vntc);
    float tempK = 1.0f / (1.0f / NTC_T0 + log(rntc / NTC_R0) / NTC_BETA);
    return tempK - 273.15f;
}

