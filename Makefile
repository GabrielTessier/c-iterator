
CC=gcc
LD=gcc

BUILD=release

CFLAGS=-O0
LDFLAGS=-O0

ifeq ($(BUILD), debug)
	CFLAGS += -Wall -Wextra -g
	LDFLAGS +=
endif

BIN_DIR=bin
OBJ_DIR=obj
SRC_DIR=.

.PHONY: all clean

all: makedir $(BIN_DIR)/test

$(BIN_DIR)/test: $(OBJ_DIR)/iterator.o $(OBJ_DIR)/test.o
	$(LD) $^ -o $@ $(LDFLAGS)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) -c $(CFLAGS) $< -o $@

makedir:
	mkdir -p $(OBJ_DIR)
	mkdir -p $(BIN_DIR)

clean:
	rm -rf $(BIN_DIR) $(OBJ_DIR)
