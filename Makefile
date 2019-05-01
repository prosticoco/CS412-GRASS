SRCDIR   = src
BINDIR   = bin
INCLUDES = include
OBJS = $(BINDIR)/utils.o $(BINDIR)/cmd.o $(BINDIR)/ftp.o $(BINDIR)/exit.o 

CC=g++
CFLAGS=-Wall -Wextra -g -fno-stack-protector -fsanitize=undefined -pthread -z execstack  -std=gnu11 -I $(INCLUDES)/ -m32 
DEPS = $(wildcard $(INCLUDES)/%.h)

all: $(BINDIR)/utils.o $(BINDIR)/exit.o $(BINDIR)/ftp.o $(BINDIR)/cmd.o $(BINDIR)/client  $(BINDIR)/server $(DEPS)

$(BINDIR):
	mkdir -p $(BINDIR)

$(BINDIR)/ftp.o : $(SRCDIR)/ftp.cpp $(INCLUDES)/ftp.h
		$(CC) -c $(CFLAGS) $< -o $@

$(BINDIR)/cmd.o : $(SRCDIR)/cmd.cpp $(INCLUDES)/cmd.h
		$(CC) -c $(CFLAGS) $< -o $@ 

$(BINDIR)/exit.o : $(SRCDIR)/exit.cpp $(INCLUDES)/exit.h
		$(CC) -c $(CFLAGS) $< -o $@ 

$(BINDIR)/utils.o : $(SRCDIR)/utils.cpp $(INCLUDES)/utils.h
		$(CC) -c $(CFLAGS) $< -o $@ 

$(BINDIR)/client: $(SRCDIR)/client.cpp $(OBJS)
	$(CC) $(CFLAGS) $< -o $@ $(OBJS)

$(BINDIR)/server: $(SRCDIR)/server.cpp $(OBJS)
	$(CC) $(CFLAGS) $< -o $@ $(OBJS)

.PHONY: clean
clean:
	rm -f $(BINDIR)/client $(BINDIR)/server $(BINDIR)/servercpp $(BINDIR)/clientcpp $(BINDIR)/*.o 
