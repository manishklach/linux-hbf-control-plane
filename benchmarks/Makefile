CC = gcc
CFLAGS = -Wall -O2 -g
LDLIBS = -lnuma -lpthread

all: sequential-access random-access promote-pressure memory-pressure

sequential-access: sequential-access.c
	$(CC) $(CFLAGS) -o $@ $< $(LDLIBS)

random-access: random-access.c
	$(CC) $(CFLAGS) -o $@ $< $(LDLIBS)

promote-pressure: promote-pressure.c
	$(CC) $(CFLAGS) -o $@ $< $(LDLIBS)

memory-pressure: memory-pressure.c
	$(CC) $(CFLAGS) -o $@ $< $(LDLIBS)

clean:
	rm -f sequential-access random-access promote-pressure memory-pressure
