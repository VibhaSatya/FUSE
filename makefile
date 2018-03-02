# define the C compiler to use
CC = gcc -Wall

# define global compile-time flags
CFLAGS = `pkg-config fuse3 --cflags --libs`

# define sources
FSTR_SRCS = driver_test.c disk.c 

BIN_DIR = .
FSTR_OBJS = $(FSTR_SRCS:.c=.o)
FSTR_TARGET = fstr

$(FSTR_TARGET): $(FSTR_OBJS)
	mkdir -p $(BIN_DIR)
	$(CC) -o $(BIN_DIR)/$@ $^ $(CFLAGS)
	rm -f $^

.PHONY: clean

clean:
	rm -rf $(FSTR_OBJS) $(BIN_DIR)



