
CC = gcc
CXX = g++
CFLAGS ?= -Wall
CXXFLAGS ?= -Wall
RM ?= rm -f

ifeq ($(OS),Windows_NT)
	LDFLAGS ?= -static
endif


.PHONY: all clean

all: c-example cpp-example

clean:
	$(RM) c-example cpp-example

c-example: c-example.c ../include/acutest.h
	$(CC) -I../include $(CPPFLAGS) $(CFLAGS) $(LDFLAGS) $< -o $@

cpp-example: cpp-example.cc ../include/acutest.h
	$(CXX) -I../include $(CPPFLAGS) $(CXXFLAGS) $(LDFLAGS) $< -o $@
