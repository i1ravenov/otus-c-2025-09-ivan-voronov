
all: cp1251_to_utf8

cp1251_to_utf8: cp1251_to_utf8.c
	$(CC) $^ -o $@ -Wall -Wextra -Wpedantic -std=c11

clean:
	rm -f cp1251_to_utf8 core

.PHONY: all clean
