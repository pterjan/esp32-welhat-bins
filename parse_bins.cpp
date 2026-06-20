#include "parse_bins.h"
#include <string.h>

#ifndef ARDUINO
// Mock Serial for testing
#include <stdio.h>
class SerialMock {
public:
    void print(const char* s) { printf("%s", s); }
    void println(const char* s) { printf("%s\n", s); }
    void println(long i) { printf("%ld\n", i); }
    void println() { printf("\n"); }
};
static SerialMock Serial;
#else
#include <Arduino.h>
#endif

static time_t parse_bin_date(char *buf, const char *service_name, const char *color) {
  time_t t_bin = 0;
  char *bin_date = strstr(buf, service_name);
  if (bin_date) {
    char *td = strstr(bin_date, "<td>");
    if (td) {
      bin_date = td + 4;
      char *e = strstr(bin_date, "</td>");
      if (e) {
        char old_char = e[0];
        e[0] = '\0';
        Serial.print("Next ");
        Serial.print(color);
        Serial.print(" bin: ");
        Serial.println(bin_date);
        struct tm t;
        memset(&t, 0, sizeof(t));
        strptime(bin_date, "%A %d %B %Y", &t);
        t_bin = mktime(&t);
        e[0] = old_char;
      }
    }
  }
  return t_bin;
}

void update_bins_from_line(char *buf, struct bins *b) {
  time_t t_blue = parse_bin_date(buf, "Recycling Collection Service", "blue");
  time_t t_black = parse_bin_date(buf, "Domestic Waste Collection Service", "black");
  time_t t_brown = parse_bin_date(buf, "Garden Waste Collection Service", "brown");

  b->next_date = t_black;
  if (t_blue > 0 && (b->next_date == 0 || t_blue < b->next_date)) {
    b->next_date = t_blue;
  }
  if (t_brown > 0 && (b->next_date == 0 || t_brown < b->next_date)) {
    b->next_date = t_brown;
  }

  Serial.print("Next collection: ");
  Serial.println(b->next_date);

  b->black = (t_black > 0 && t_black == b->next_date);
  b->blue = (t_blue > 0 && t_blue == b->next_date);
  b->brown = (t_brown > 0 && t_brown == b->next_date);
}
