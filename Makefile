.SUFFIXES : .c .o
CC = gcc
CFLAGS = -Wall \
         -Wextra \
         -Wpointer-arith \
         -Wcast-align \
         -Wwrite-strings \
         -Wswitch-default \
         -Wunreachable-code \
         -Winit-self \
         -Wmissing-field-initializers \
         -Wno-unknown-pragmas \
         -Wundef \
         -Wconversion \
         -Werror \
         -O3
LIBS = -lwiringPi \
       -lasan \
       -fsanitize=address \
       -static-libasan
INCLUDES = -I./reedsolomon-c
OBJS = nand-core.o nand-write.o nand-read.o nand-erase.o nand-utils.o main.o rs.o
TARGET = a.out


all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) -o $@ $^ $(LIBS)

rs.o: reedsolomon-c/rs.c
	$(CC) -O3 $(INCLUDES) -c -o $@ $^ $(LIBS)

main.o: main.c
	$(CC) $(CFLAGS) $(INCLUDES) -c -o $@ $^

nand-%.o: nand-%.c
	$(CC) $(CFLAGS) $(INCLUDES) -c -o $@ $<

check:
	cppcheck *.[ch]

clean:
	rm -rf *.o
	rm -rf $(TARGET)
