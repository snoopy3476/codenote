CC=gcc
MAKE=make

TARGET=codenote

SRC:=$(wildcard *.c)
OBJS:=$(SRC:.c=.o)
LIBS:=-lgcrypt

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $^ -o $@ $(LIBS)

%.o: %.c %.h
	$(CC) $(CFLAGS) -c $< -o $@ $(LIBS)


clean:
	rm -f $(OBJS) $(TARGET)
