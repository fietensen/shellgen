CC=gcc
CFLAGS=-Wall -I./include -g -fsanitize=address
SRC=$(shell find . -name "*.c")
PROJ=./shellgen

all:
	$(CC) $(CFLAGS) $(SRC) -o $(PROJ)
