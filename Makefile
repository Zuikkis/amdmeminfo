CC=gcc
CFLAGS=-O3 -Wall
LDFLAGS=-lpci

amdmeminfo: amdmeminfo.c
	gcc -O3 -o $@ $^ -lpci

clean:
	rm -f amdmeminfo *.o
