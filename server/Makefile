
ifndef CROSS_COMPILE
	CC = cc
else
	CC = $(CROSS_COMPILE)gcc
endif

CFLAGS += -Wall
CFLAGS += -Werror

all: clean default

default: aesdsocket.c
	$(CC) $(CFLAGS) aesdsocket.c -o aesdsocket

clean:
	rm -f aesdsocket
