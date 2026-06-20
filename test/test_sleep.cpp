#include <gtest/gtest.h>
#include <string.h>
#include <time.h>

#include "../sleep_bins.h"
#include "mocks/Arduino.h"

class SleepTest : public ::testing::Test {
protected:
    void SetUp() override {
        reset_mocks();
    }
};

TEST_F(SleepTest, GetLocalTimeFails) {
    struct bins b;
    b.next_date = 0;

    mock_getLocalTime_fail = true;

    sleep_if_not_tonight(&b);

    // Should return early, not sleep
    EXPECT_FALSE(mock_deep_sleep_called);
    EXPECT_EQ(mock_delay_ms, 0);
}

TEST_F(SleepTest, TomorrowIsCollectionDay) {
    struct bins b;

    // Set mock local time to Dec 10, 2025
    memset(&mock_local_time, 0, sizeof(mock_local_time));
    mock_local_time.tm_year = 2025 - 1900;
    mock_local_time.tm_mon = 11; // December
    mock_local_time.tm_mday = 10;
    mock_local_time.tm_hour = 12;
    mock_local_time.tm_min = 0;
    mock_local_time.tm_sec = 0;

    // The target next collection is tomorrow (Dec 11, 2025 at 00:00:00)
    struct tm target_tm = mock_local_time;
    target_tm.tm_mday += 1;
    target_tm.tm_hour = 0;
    target_tm.tm_min = 0;
    target_tm.tm_sec = 0;
    b.next_date = mktime(&target_tm);

    sleep_if_not_tonight(&b);

    // Should NOT go to sleep
    EXPECT_FALSE(mock_deep_sleep_called);
    EXPECT_EQ(mock_delay_ms, 0);
}

TEST_F(SleepTest, TomorrowIsNotCollectionDay) {
    struct bins b;

    // Set mock local time to Dec 10, 2025
    memset(&mock_local_time, 0, sizeof(mock_local_time));
    mock_local_time.tm_year = 2025 - 1900;
    mock_local_time.tm_mon = 11; // December
    mock_local_time.tm_mday = 10;
    mock_local_time.tm_hour = 12;
    mock_local_time.tm_min = 0;
    mock_local_time.tm_sec = 0;

    // The target next collection is later (Dec 17, 2025)
    struct tm target_tm = mock_local_time;
    target_tm.tm_mday += 7;
    target_tm.tm_hour = 0;
    target_tm.tm_min = 0;
    target_tm.tm_sec = 0;
    b.next_date = mktime(&target_tm);

    sleep_if_not_tonight(&b);

    // Should go to sleep (delay 30000 + 1000 from sleep_until_5pm)
    EXPECT_TRUE(mock_deep_sleep_called);
    EXPECT_EQ(mock_delay_ms, 31000);

    // Calculate expected sleep time
    // Next 5pm is Dec 10, 2025 17:00:00
    struct tm next_5pm = mock_local_time;
    next_5pm.tm_hour = 17;
    time_t t_now = mktime(&mock_local_time);
    time_t t_next_5pm = mktime(&next_5pm);
    uint64_t expected_sleep_time = (t_next_5pm - t_now) * 1000000L;

    EXPECT_EQ(mock_sleep_time, expected_sleep_time);
}

TEST_F(SleepTest, TomorrowIsNotCollectionDayAfter5PM) {
    struct bins b;

    // Set mock local time to Dec 10, 2025, 18:00 (after 5pm)
    memset(&mock_local_time, 0, sizeof(mock_local_time));
    mock_local_time.tm_year = 2025 - 1900;
    mock_local_time.tm_mon = 11; // December
    mock_local_time.tm_mday = 10;
    mock_local_time.tm_hour = 18;
    mock_local_time.tm_min = 0;
    mock_local_time.tm_sec = 0;

    // The target next collection is later (Dec 17, 2025)
    struct tm target_tm = mock_local_time;
    target_tm.tm_mday += 7;
    target_tm.tm_hour = 0;
    target_tm.tm_min = 0;
    target_tm.tm_sec = 0;
    b.next_date = mktime(&target_tm);

    sleep_if_not_tonight(&b);

    // Should go to sleep
    EXPECT_TRUE(mock_deep_sleep_called);

    // Next 5pm is Dec 11, 2025 17:00:00
    struct tm next_5pm = mock_local_time;
    next_5pm.tm_mday += 1;
    next_5pm.tm_hour = 17;
    time_t t_now = mktime(&mock_local_time);
    time_t t_next_5pm = mktime(&next_5pm);
    uint64_t expected_sleep_time = (t_next_5pm - t_now) * 1000000L;

    EXPECT_EQ(mock_sleep_time, expected_sleep_time);
}
