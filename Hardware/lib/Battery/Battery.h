#pragma once

#define BAT_TEMP_PIN   35   // NTC thermistor: 3.3V → 10kΩ → GPIO35 → NTC → GND

namespace Battery {
    void  init();
    int   getPercentage();
    float getVoltage();
    float getTemperature();   // °C
}
