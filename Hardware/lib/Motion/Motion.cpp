#include "Motion.h"
#include <Wire.h>
#include <Arduino.h>

#define ADXL345_ADDR   0x53
#define REG_POWER_CTL  0x2D
#define REG_DATA_FMT   0x31
#define REG_THRESH_TAP 0x1D
#define REG_DUR        0x21
#define REG_TAP_AXES   0x2A
#define REG_INT_MAP    0x2F
#define REG_INT_ENABLE 0x2E
#define REG_INT_SOURCE 0x30

static void writeReg(uint8_t reg, uint8_t value) {
    Wire.beginTransmission(ADXL345_ADDR);
    Wire.write(reg);
    Wire.write(value);
    Wire.endTransmission();
}

static uint8_t readReg(uint8_t reg) {
    Wire.beginTransmission(ADXL345_ADDR);
    Wire.write(reg);
    Wire.endTransmission(false);
    Wire.requestFrom((uint8_t)ADXL345_ADDR, (uint8_t)1);
    if (Wire.available()) return Wire.read();
    return 0;
}

void Motion::init() {
    writeReg(REG_POWER_CTL,  0x00); // standby
    writeReg(REG_DATA_FMT,   0x0B); // full resolution ±16g
    writeReg(REG_THRESH_TAP, 0x18); // ~1.5g tap threshold
    writeReg(REG_DUR,        0x10); // 10ms tap duration
    writeReg(REG_TAP_AXES,   0x07); // X, Y, Z
    writeReg(REG_INT_MAP,    0x00); // all interrupts → INT1
    writeReg(REG_INT_ENABLE, 0x40); // single tap interrupt
    writeReg(REG_POWER_CTL,  0x08); // measurement mode
    delay(100);
    readReg(REG_INT_SOURCE);        // clear startup interrupt
}

void Motion::enableWakeOnMotion() {
    readReg(REG_INT_SOURCE);        // clear pending interrupt
    esp_sleep_enable_ext0_wakeup((gpio_num_t)MOTION_INT_PIN, 1);
}
