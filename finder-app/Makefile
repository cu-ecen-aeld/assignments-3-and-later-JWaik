# Planning
# Cross platform
# if not specified 'CROSS_COMPILE', use native build
# if specified, use cross compiler that pass through this makefile
# Usage: make CROSS_COMPILE=aarch64-none-linux-gnu- (from assignment2 description)

ifndef CROSS_COMPILE
  CC = cc
else
  CC = $(CROSS_COMPILE)gcc
endif

writer: writer.o
	$(CC) writer.c -o writer

writer.o: writer.c
	$(CC) -c writer.c -o writer.o

clean:
	rm -f writer writer.o
