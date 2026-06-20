CXX = g++
CXXFLAGS = -std=c++14 -I. -I.. -I./mocks -Wall
LDFLAGS = -L/usr/lib/x86_64-linux-gnu -lgtest -lgtest_main -pthread

all: test_bins

test_bins: test_bins.o test_sleep.o parse_bins.o sleep_bins.o Arduino.o
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

test_bins.o: test_bins.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

test_sleep.o: test_sleep.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

parse_bins.o: ../parse_bins.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

sleep_bins.o: ../sleep_bins.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

Arduino.o: mocks/Arduino.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

clean:
	rm -f *.o test_bins
