SRCDIR   = src
BINDIR   = bin
INCLUDES = include
OBJS = cmd.o

CC=g++
CFLAGS=-Wall -Wextra -g -fno-stack-protector -z execstack -lpthread -std=gnu11 -I $(INCLUDES)/ -m32
DEPS = $(wildcard $(INCLUDES)/%.h)

all: $(BINDIR)/client $(BINDIR)/server $(DEPS)

cmd.o : $(SRCDIR)/cmd.cpp $(INCLUDES)/cmd.h
		$(CC) $(CFLAGS) -c $(SRCDIR)/cmd.cpp

$(BINDIR)/client: $(SRCDIR)/client.cpp $(OBJS)
	$(CC) $(CFLAGS) $< -o $@ 

$(BINDIR)/server: $(SRCDIR)/server.cpp $(OBJS)
	$(CC) $(CFLAGS) $< -o $@

.PHONY: clean
clean:
	rm -f $(BINDIR)/client $(BINDIR)/server $(BINDIR)/servercpp $(BINDIR)/clientcpp
