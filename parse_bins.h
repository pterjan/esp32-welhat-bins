#ifndef PARSE_BINS_H
#define PARSE_BINS_H

#include <time.h>

struct bins {
  bool blue;
  bool brown;
  bool black;
  time_t next_date;
};

void update_bins_from_line(char *buf, struct bins *b);

#endif
