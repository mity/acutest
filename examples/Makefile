
CC = gcc
CXX = g++
CFLAGS ?= -Wall
CXXFLAGS ?= -Wall
LDFLAGS ?= -static


all: c-example cpp-example

c-example: examples/c-example.c include/cutest.h
	$(CC) -Iinclude $(CPPFLAGS) $(CFLAGS) $(LDFLAGS) $^ -o $@

cpp-example: examples/cpp-example.cc include/cutest.h
	$(CXX) -Iinclude $(CPPFLAGS) $(CXXFLAGS) $(LDFLAGS) $^ -o $@
