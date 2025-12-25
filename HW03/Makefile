AS      = nasm
CC      = clang

ASFLAGS = -g -f macho64
CFLAGS  = -g -arch x86_64
LDFLAGS = -target x86_64-apple-macos10.15

TARGET  = asm-prog
OBJS    = main.o

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(LDFLAGS) $^ -o $@

main.o: main.asm
	$(AS) $(ASFLAGS) $< -o $@

clean:
	rm -f $(TARGET) *.o

.PHONY: all clean
