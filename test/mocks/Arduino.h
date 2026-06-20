#pragma once
#include <string>
#include <stdint.h>
#include <stdio.h>

class String {
public:
    std::string s;
    void reserve(int n) { s.reserve(n); }
    void concat(const char* str) { s += str; }
    void concat(int i) { s += std::to_string(i); }
    void concat(const String& str) { s += str.s; }
    int length() const { return s.length(); }
    operator const char*() const { return s.c_str(); }
};

class SerialMock {
public:
    void print(const char* s) {}
    void print(int i) {}
    void print(long i) {}
    void print(String s) {}
    void println(const char* s) {}
    void println(int i) {}
    void println(long i) {}
    void println(String s) {}
    void println(const struct tm* timeinfo, const char* format) {}
    void println() {}
    void begin(int b) {}
    void flush() {}
    void printf(const char* fmt, ...) {}
};
extern SerialMock Serial;

unsigned long millis();
void delay(int ms);
bool getLocalTime(struct tm * info, uint32_t ms=5000);

#define WL_CONNECTED 3

typedef int esp_sleep_wakeup_cause_t;
#define ESP_SLEEP_WAKEUP_EXT0 0
#define ESP_SLEEP_WAKEUP_EXT1 1
#define ESP_SLEEP_WAKEUP_TIMER 2
#define ESP_SLEEP_WAKEUP_TOUCHPAD 3
#define ESP_SLEEP_WAKEUP_ULP 4
esp_sleep_wakeup_cause_t esp_sleep_get_wakeup_cause();
void esp_sleep_enable_timer_wakeup(uint64_t time_in_us);
void esp_deep_sleep_start();

#define T0 0
void esp_sleep_enable_touchpad_wakeup();
void touchSleepWakeUpEnable(int pin, int threshold);

// Mock state variables for asserting in tests
extern bool mock_getLocalTime_fail;
extern struct tm mock_local_time;
extern uint64_t mock_sleep_time;
extern bool mock_deep_sleep_called;
extern int mock_delay_ms;

void reset_mocks();
