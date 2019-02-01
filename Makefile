PROGS          = ashmon
CC	           = gcc
CFLAGS	       = -Wall -Wno-unused-result
CFLAGS_RELEASE = -O3
CFLAGS_DEBUG   = -g3 -O0
LIBS	       = -lX11 -pthread
SRCS           = src/*.c

default: $(PROGS)

all: $(PROGS) debug

$(PROGS):
	@mkdir -p bin
	$(CC) $(CFLAGS) $(CFLAGS_RELEASE) -o bin/$@ $(SRCS) $(LIBS)

debug:
	@mkdir -p bin
	$(CC) $(CFLAGS) $(CFLAGS_DEBUG) -o bin/$@ $(SRCS) $(LIBS)

clean:
	rm -fr bin
	