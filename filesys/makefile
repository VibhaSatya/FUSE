# define the C compiler to use
CC = gcc -Wall

# define global compile-time flags
CFLAGS = `pkg-config fuse3 --cflags --libs`

# define sources
SRCS = driver_test.c disk.c utils.c namei.c

BIN_DIR = .
OBJS = $(SRCS:.c=.o)
TARGET = driver

$(TARGET): $(OBJS)
	mkdir -p $(BIN_DIR)
	$(CC) -o $(BIN_DIR)/$@ $^ $(CFLAGS)
	rm -f $^

.PHONY: clean

clean:
	rm -rf $(OBJS) $(BIN_DIR)



