CC = clang
AR = ar
ARFLAGS = rcs

SRCS = cJSON.c
OBJS = $(SRCS:.c=.o)

LIB = libcjson.a
LIBS = -lcurl

CFLAGS = -Wall -Wextra -Wpedantic -std=c11

all: $(LIB) app

$(LIB): $(OBJS)
	$(AR) $(ARFLAGS) $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

app: main.o $(LIB)
	$(CC) main.o $(LIB) $(LIBS) -o app

clean:
	rm -f *.o *.a app
