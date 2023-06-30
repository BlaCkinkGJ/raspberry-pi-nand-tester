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
OBJS = nand-core.o \
       nand-write.o \
       nand-read.o \
       nand-erase.o \
       nand-utils.o \
       nand-oob.o \
       nand-hamming256.o \
       main.o
TARGET = a.out
LIBRARY_TARGET = libnand.a

USE_DEBUG = 0
ifeq ($(USE_DEBUG), 1)
CFLAGS += -g -pg -DDEBUG
LIBS += -lasan \
        -fsanitize=address \
        -static-libasan
endif

ifeq ($(PREFIX),)
PREFIX := /usr/local
endif

all: $(TARGET)

install: $(LIBRARY_TARGET)
	install -d $(DESTDIR)$(PREFIX)/lib/
	install -m 644 $(LIBRARY_TARGET) $(DESTDIR)$(PREFIX)/lib
	install -d $(DESTDIR)$(PREFIX)/include/nand
	install -m 644 *.h $(DESTDIR)$(PREFIX)/include/nand

$(TARGET): $(OBJS)
	$(CC) -o $@ $^ $(LIBS)

$(LIBRARY_TARGET): $(OBJS)
	$(AR) $(ARFLAGS) $@ $^

main.o: main.c
	$(CC) $(CFLAGS) $(INCLUDES) -c -o $@ $^

nand-%.o: nand-%.c
	$(CC) $(CFLAGS) $(INCLUDES) -c -o $@ $<

check:
	cppcheck *.[ch]

clean:
	rm -rf *.o
	rm -rf $(TARGET) $(LIBRARY_TARGET)
