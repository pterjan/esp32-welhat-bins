#include "sleep_bins.h"

#include <Arduino.h>
#include <string.h>

void sleep_until_5pm() {
  struct tm now, next;
  time_t t_now, t_next;
  if (!getLocalTime(&now)) {
    Serial.println("Failed to obtain time");
    return;
  }
  Serial.println(&now, "Current time is %A %d %B %Y %H:%M:%S");
  memcpy(&next, &now, sizeof(now));
  if (now.tm_hour >= 17) {
    next.tm_mday += 1;
  }
  next.tm_hour = 17;
  next.tm_min = 0;
  t_next = mktime(&next);
  Serial.print("Sleeping until next 5pm: ");
  Serial.println(&next, "%A %d %B %Y %H:%M");

  t_now = mktime(&now);

  esp_sleep_enable_timer_wakeup((t_next - t_now) * 1000000L);
  touchSleepWakeUpEnable(T0, 40);
  esp_sleep_enable_touchpad_wakeup();
  Serial.println("Going to sleep now");
  delay(1000);
  Serial.flush();
  esp_deep_sleep_start();
}

void sleep_if_not_tonight(struct bins *b) {
  struct tm now, next;
  if (!getLocalTime(&now)) {
    Serial.println("Failed to obtain time");
    return;
  }
  memcpy(&next, &now, sizeof(now));
  next.tm_mday += 1;
  next.tm_sec = 0;
  next.tm_min = 0;
  next.tm_hour = 0;
  mktime(&next);

  char s[32];
  strftime(s, 32, "%A %d %B %Y", &next);
  Serial.print("Tomorrow: ");
  Serial.println(s);

  if (mktime(&next) != b->next_date) {

    delay(30000);
    sleep_until_5pm();
  }
}


void sleep_at_night() {
  struct tm now;
  if (!getLocalTime(&now)) {
    Serial.println("Failed to obtain time");
    return;
  }
  if (now.tm_hour == 23) {
    sleep_until_5pm();
  }
}
