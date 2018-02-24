PROGS = ashmon

CC	= gcc
CFLAGS	= -Wall
LIBS	= -lX11 -pthread

all:	$(PROGS)

clean:
	rm -f $(PROGS)

distclean: clean
	rm -f *~ \#* core

ashmon: ashmon.c
	$(CC) $(CFLAGS) window.c nic.c -o $@ $< $(LIBS)
