CC = gcc
CFLAGS = -std=c17 -Wall -Wextra -Wpedantic -pthread -mbmi2
LDLIBS = -lm
DEBUG_FLAGS = -g -O0
RELEASE_FLAGS = -O3

BIN = bot
SRC = main.c

.PHONY: all debug release clean

all: debug

debug: CFLAGS += $(DEBUG_FLAGS)
debug: $(BIN)

release: CFLAGS += $(RELEASE_FLAGS)
release: $(BIN)

$(BIN): $(SRC)
	$(CC) $(CFLAGS) -o $(BIN) $(SRC) $(LDLIBS)

clean:
	rm -f $(BIN)
