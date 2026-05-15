#pragma once

#define MOTION_INT_PIN 33

namespace Motion {
    void init();
    void enableWakeOnMotion();  // call before deep sleep
}
