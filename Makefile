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
INCLUDES = 
SRCS = nand-core.c nand-write.c nand-read.c nand-erase.c nand-utils.c main.c
TARGET = a.out


all: $(TARGET)

$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ $^ $(LIBS)

check:
	cppcheck *.[ch]

clean:
	rm -rf *.o
	rm -rf $(TARGET)
