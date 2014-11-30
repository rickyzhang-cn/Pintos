CC=gcc
CFLAGS=-g
LDFLAGS=

SOURCES=main.c map.c wc.c
OBJECTS=$(SOURCES:.c=.o)
EXECUTABLE=main.out map.out wc.out 

all: $(EXECUTABLE)

main.out:
	$(CC) $(CFLAGS) main.c -o $@
map.out:
	$(CC) $(CFLAGS) map.c -o $@
wc.out:
	$(CC) $(CFLAGS) wc.c -o $@

rule: loop
	@for i in 1 2 3 4 5; \
	do \
		echo $$i; \
	done

loop:
	@for src in $(SOURCES); \
	do \
		echo $$src; \
	done
	@CC=loop_gcc \
	echo $$CC

clean:
	rm $(EXECUTABLE)
