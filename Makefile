CC=gcc
CFLAGS=-Wall -I./include# -g
SRC=$(shell find . -name "*.c")
PROJ=./shellgen

all:
	$(CC) $(CFLAGS) $(SRC) -o $(PROJ)
