SRCDIR   = src
BINDIR   = bin
INCLUDES = include

CCC=g++
CC=gcc
CFLAGS=-Wall -Wextra -g -fno-stack-protector -z execstack -lpthread -std=gnu11 -I $(INCLUDES)/ -m32
DEPS = $(wildcard $(INCLUDES)/%.h)

all: $(BINDIR)/clientcpp $(BINDIR)/client $(BINDIR)/server $(BINDIR)/servercpp $(DEPS)

$(BINDIR)/clientcpp: $(SRCDIR)/client.cpp
	$(CCC) $(CFLAGS) $< -o $@

$(BINDIR)/client: $(SRCDIR)/client.c
	$(CC) $(CFLAGS) $< -o $@

$(BINDIR)/server: $(SRCDIR)/server.c
	$(CC) $(CFLAGS) $< -o $@

$(BINDIR)/servercpp: $(SRCDIR)/server.cpp
	$(CCC) $(CFLAGS) $< -o $@



.PHONY: clean
clean:
	rm -f $(BINDIR)/client $(BINDIR)/server $(BINDIR)/servercpp $(BINDIR)/clientcpp
