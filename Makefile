PROGS = ashmon

CC	= gcc
CFLAGS	= -Wall
LIBS	= -lX11 -pthread

default: $(PROGS)

all: $(PROGS) debug

$(PROGS): ashmon.c
	$(CC) $(CFLAGS) window.c nic.c -o $@ $< $(LIBS)

debug: ashmon.c
	$(CC) $(CFLAGS) -g window.c nic.c -o $@ $< $(LIBS)

clean:
	rm -f $(PROGS) debug

distclean: clean
	rm -f *~ \#* core
	