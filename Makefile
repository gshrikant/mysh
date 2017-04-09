# sh

CFLAGS=-Wall -W -Os -g
CC = gcc
C_INCLUDE_PATH = ./includes

all : sh.c
	@$(CC) -I$(C_INCLUDE_PATH) $(CFLAGS) linenoise.c sh.c  -o sh

clean:
	@rm -rf *.o sh
