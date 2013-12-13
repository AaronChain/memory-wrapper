##
#  Makefile
#
#  Author: rp <rp@meetrp.com>
#
##


# your filename (also the final binary name) appears here
BIN = test

RM = rm
CC = gcc

CFLAGS = -g -Wall

# directory where this library will be placed
IDIR = .
CFLAGS += -I$(IDIR) 

# for Backtrace
CFLAGS += -rdynamic 

# all files of this library
DEPS = fmc_common.h fmc_log.h fmc.h
OBJ = fmc_log.o fmc.o

# your file is compiled here
OBJ += $(BIN).o

# prepare the object files
%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

# prepare the final binary
$(BIN): $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS)

.PHONY: clean

clean:
	$(RM) -f *.o *~ core $(BIN)
