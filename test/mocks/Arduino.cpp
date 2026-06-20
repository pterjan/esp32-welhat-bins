#include <time.h>
#include "Arduino.h"

SerialMock Serial;

bool mock_getLocalTime_fail = false;
struct tm mock_local_time;
uint64_t mock_sleep_time = 0;
bool mock_deep_sleep_called = false;
int mock_delay_ms = 0;

void reset_mocks() {
    mock_getLocalTime_fail = false;
    mock_sleep_time = 0;
    mock_deep_sleep_called = false;
    mock_delay_ms = 0;
}

bool getLocalTime(struct tm * info, uint32_t ms) {
    if (mock_getLocalTime_fail) return false;
    *info = mock_local_time;
    return true;
}

void delay(int ms) {
    mock_delay_ms += ms;
}

void esp_sleep_enable_timer_wakeup(uint64_t time_in_us) {
    mock_sleep_time = time_in_us;
}

void esp_deep_sleep_start() {
    mock_deep_sleep_called = true;
}

void esp_sleep_enable_touchpad_wakeup() {}
void touch_pad_clear_status() {}
void touchSleepWakeUpEnable(int pin, int threshold) {}

