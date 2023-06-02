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
LIBS = -lwiringPi
INCLUDES = 
SRCS = nand.c main.c
TARGET = a.out


all: $(TARGET)

$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ $^ $(LIBS)

clean:
	rm -rf $(SRCS)
	rm -rf $(OBJS)
