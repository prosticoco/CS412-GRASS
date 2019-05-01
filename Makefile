SRCDIR   = src
BINDIR   = bin
INCLUDES = include
OBJS = utils.o cmd.o ftp.o exit.o  path.o

CC=g++
CFLAGS=-Wall -Wextra -g -fno-stack-protector -fsanitize=undefined -pthread -z execstack  -std=gnu11 -I $(INCLUDES)/ -m32 
DEPS = $(wildcard $(INCLUDES)/%.h)

all: utils.o exit.o ftp.o cmd.o path.o $(BINDIR)/client  $(BINDIR)/server $(DEPS)
 
path.o : $(SRCDIR)/path.cpp $(INCLUDES)/path.h
		$(CC) -c $(CFLAGS) $< -o $@

ftp.o : $(SRCDIR)/ftp.cpp $(INCLUDES)/ftp.h
		$(CC) -c $(CFLAGS) $< -o $@

cmd.o : $(SRCDIR)/cmd.cpp $(INCLUDES)/cmd.h
		$(CC) -c $(CFLAGS) $< -o $@ 

exit.o : $(SRCDIR)/exit.cpp $(INCLUDES)/exit.h
		$(CC) -c $(CFLAGS) $< -o $@ 

utils.o : $(SRCDIR)/utils.cpp $(INCLUDES)/utils.h
		$(CC) -c $(CFLAGS) $< -o $@ 

$(BINDIR)/client: $(SRCDIR)/client.cpp $(OBJS)
	$(CC) $(CFLAGS) $< -o $@ $(OBJS)

$(BINDIR)/server: $(SRCDIR)/server.cpp $(OBJS)
	$(CC) $(CFLAGS) $< -o $@ $(OBJS)

.PHONY: clean
clean:
	rm -f $(BINDIR)/client $(BINDIR)/server $(BINDIR)/servercpp $(BINDIR)/clientcpp 
