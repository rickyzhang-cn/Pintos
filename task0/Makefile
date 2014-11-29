CC=gcc
CFLAGS=-g
LDFLAGS=

SOURCES=main.c map.c wc.c
OBJECTS=$(SOURCES:.c=.o)
EXECUTABLE=main map wc 

all: $(EXECUTABLE)

main:
	$(CC) $(CFLAGS) main.c -o $@.out
map:
	$(CC) $(CFLAGS) map.c -o $@.out
wc:
	$(CC) $(CFLAGS) wc.c -o $@.out

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
	echo $CC
clean:
	rm $(EXECUTABLE)
