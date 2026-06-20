#include <gtest/gtest.h>
#include <string.h>

#include "../parse_bins.h"

TEST(BinsTest, ParseBinsBlueAndBlackOnly) {
  struct bins b;
  b.next_date = 0;
  char buf[] = "<tr><td>Domestic Waste Collection Service</td><td>Wednesday 10 December 2025</td></tr>"
               "<tr><td>Food Waste Collection Service</td><td>Wednesday 10 December 2025</td></tr>"
               "<tr><td>Recycling Collection Service</td><td>Wednesday 17 December 2025</td></tr>";
  update_bins_from_line(buf, &b);
  EXPECT_TRUE(b.black);
  EXPECT_FALSE(b.blue);
  EXPECT_FALSE(b.brown);
  // Also expect the next_date to not be zero
  EXPECT_NE(b.next_date, 0);
}

TEST(BinsTest, ParseBinsAllThree) {
  struct bins b;
  b.next_date = 0;
  char buf[] = "<tr><td>Domestic Waste Collection Service</td><td>Wednesday 17 December 2025</td></tr>"
               "<tr><td>Garden Waste Collection Service</td><td>Wednesday 10 December 2025</td></tr>"
               "<tr><td>Recycling Collection Service</td><td>Wednesday 10 December 2025</td></tr>";
  update_bins_from_line(buf, &b);
  EXPECT_FALSE(b.black);
  EXPECT_TRUE(b.blue);
  EXPECT_TRUE(b.brown);
}

TEST(BinsTest, ParseBinsEmptyBuf) {
  struct bins b;
  b.next_date = 0;
  char buf[] = "";
  update_bins_from_line(buf, &b);
  EXPECT_FALSE(b.black);
  EXPECT_FALSE(b.blue);
  EXPECT_FALSE(b.brown);
}

TEST(BinsTest, ParseBinsMissingTableData) {
  struct bins b;
  b.next_date = 0;
  char buf[] = "<tr><td>Domestic Waste Collection Service</td></tr>"
               "<tr><td>Recycling Collection Service</td></tr>"
               "<tr><td>Garden Waste Collection Service</td></tr>";
  update_bins_from_line(buf, &b);
  EXPECT_FALSE(b.black);
  EXPECT_FALSE(b.blue);
  EXPECT_FALSE(b.brown);
}

TEST(BinsTest, ParseBinsMissingClosingTag) {
  struct bins b;
  b.next_date = 0;
  char buf[] = "<tr><tdDomestic Waste Collection Service<tdWednesday 10 December 2025<tr"
               "<tr><tdRecycling Collection Service<tdWednesday 17 December 2025<tr"
               "<tr><tdGarden Waste Collection Service<tdWednesday 10 December 2025<tr";
  update_bins_from_line(buf, &b);
  EXPECT_FALSE(b.black);
  EXPECT_FALSE(b.blue);
  EXPECT_FALSE(b.brown);
  EXPECT_EQ(b.next_date, 0);
}

TEST(BinsTest, ParseBinsInvalidDate) {
  struct bins b;
  b.next_date = 0;
  char buf[] = "<tr><td>Domestic Waste Collection Service</td><td>Not a date</td></tr>"
               "<tr><td>Recycling Collection Service</td><td>Not a date</td></tr>"
               "<tr><td>Garden Waste Collection Service</td><td>Not a date</td></tr>";
  update_bins_from_line(buf, &b);

  // Create a local tm zeroed out, call mktime, and check against its result instead of a hardcoded epoch
  struct tm t_zero;
  memset(&t_zero, 0, sizeof(t_zero));
  time_t expected_fail_time = mktime(&t_zero);

  EXPECT_EQ(b.next_date, expected_fail_time);
}

TEST(BinsTest, ParseBinsBrownFirst) {
  struct bins b;
  b.next_date = 0;
  char buf[] = "<tr><td>Domestic Waste Collection Service</td><td>Wednesday 24 December 2025</td></tr>"
               "<tr><td>Garden Waste Collection Service</td><td>Wednesday 10 December 2025</td></tr>"
               "<tr><td>Recycling Collection Service</td><td>Wednesday 17 December 2025</td></tr>";
  update_bins_from_line(buf, &b);
  EXPECT_FALSE(b.black);
  EXPECT_FALSE(b.blue);
  EXPECT_TRUE(b.brown);
}

TEST(BinsTest, ParseBinsBlackFirst) {
  struct bins b;
  b.next_date = 0;
  char buf[] = "<tr><td>Domestic Waste Collection Service</td><td>Wednesday 10 December 2025</td></tr>"
               "<tr><td>Garden Waste Collection Service</td><td>Wednesday 24 December 2025</td></tr>"
               "<tr><td>Recycling Collection Service</td><td>Wednesday 17 December 2025</td></tr>";
  update_bins_from_line(buf, &b);
  EXPECT_TRUE(b.black);
  EXPECT_FALSE(b.blue);
  EXPECT_FALSE(b.brown);
}

TEST(BinsTest, ParseBinsBlueFirst) {
  struct bins b;
  b.next_date = 0;
  char buf[] = "<tr><td>Domestic Waste Collection Service</td><td>Wednesday 17 December 2025</td></tr>"
               "<tr><td>Garden Waste Collection Service</td><td>Wednesday 24 December 2025</td></tr>"
               "<tr><td>Recycling Collection Service</td><td>Wednesday 10 December 2025</td></tr>";
  update_bins_from_line(buf, &b);
  EXPECT_FALSE(b.black);
  EXPECT_TRUE(b.blue);
  EXPECT_FALSE(b.brown);
}

TEST(BinsTest, ParseBinsSortedBlueBrownFirst) {
  struct bins b;
  b.next_date = 0;
  char buf[] = "<tr><td>Recycling Collection Service</td><td>Wednesday 10 December 2025</td></tr>"
               "<tr><td>Garden Waste Collection Service</td><td>Wednesday 10 December 2025</td></tr>"
               "<tr><td>Domestic Waste Collection Service</td><td>Wednesday 17 December 2025</td></tr>";
  update_bins_from_line(buf, &b);
  EXPECT_FALSE(b.black);
  EXPECT_TRUE(b.blue);
  EXPECT_TRUE(b.brown);
}
