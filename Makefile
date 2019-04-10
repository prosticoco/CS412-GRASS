SRCDIR   = src
BINDIR   = bin
INCLUDES = include
OBJS = cmd.o exit.o

CC=g++
CFLAGS=-Wall -Wextra -g -fno-stack-protector -z execstack -lpthread -std=gnu11 -I $(INCLUDES)/ -m32
DEPS = $(wildcard $(INCLUDES)/%.h)

all: cmd.o exit.o $(BINDIR)/client $(BINDIR)/server $(DEPS)

cmd.o : $(SRCDIR)/cmd.cpp $(INCLUDES)/cmd.h
		$(CC) -c $(CFLAGS) $< -o $@ 

exit.o : $(SRCDIR)/exit.cpp $(INCLUDES)/exit.h
		$(CC) -c $(CFLAGS) $< -o $@ 

$(BINDIR)/client: $(SRCDIR)/client.cpp $(OBJS)
	$(CC) $(CFLAGS) $< -o $@ $(OBJS)

$(BINDIR)/server: $(SRCDIR)/server.cpp $(OBJS)
	$(CC) $(CFLAGS) $< -o $@ $(OBJS)

.PHONY: clean
clean:
	rm -f $(BINDIR)/client $(BINDIR)/server $(BINDIR)/servercpp $(BINDIR)/clientcpp 
