
CC = gcc
CXX = g++
CFLAGS ?= -Wall
CXXFLAGS ?= -Wall
LDFLAGS ?= -static


all: c-example cpp-example

c-example: c-example.c ../include/cutest.h
	$(CC) -I../include $(CPPFLAGS) $(CFLAGS) $(LDFLAGS) $< -o $@

cpp-example: cpp-example.cc ../include/cutest.h
	$(CXX) -I../include $(CPPFLAGS) $(CXXFLAGS) $(LDFLAGS) $< -o $@
