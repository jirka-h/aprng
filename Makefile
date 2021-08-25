OBJS	= src/libaprng-util.o src/libaprng-sturmian_word.o src/libaprng-sturmian_generator.o src/test-sturmian_generator.o
SOURCE	= src/libaprng-util.c src/libaprng-sturmian_word.c src/libaprng-sturmian_generator.c src/test-sturmian_generator.c
HEADER	= src/libaprng-util.h src/libaprng-sturmian_word.h src/libaprng-sturmian_generator.h
OUT	= test-sturmian_generator
CC	 = gcc
CFLAGS	 = -g -Wall -Wextra -Wpedantic -O3 -fsanitize=undefined

all: $(OBJS)
	$(CC) $(OBJS) -o $(OUT) $(CFLAGS)

%.o: %.c $(HEADER)
	$(CC) -c -o $@ $< $(CFLAGS)

test: all
	./$(OUT) --gen=trib -s -o trib.test -n10000
	diff trib.test tests/trib.txt

clean:
	rm -f $(OBJS) $(OUT) trib.test

