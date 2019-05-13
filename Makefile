CC := gcc
MAKE := make
override CFLAGS += -Wall

TARGET := codenote
SRC := codenote.c noteio.c
OBJS := $(SRC:.c=.o)
LIBS := -lgcrypt

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $^ -o $@ $(LIBS)

codenote.o: codenote.c noteio.h
	$(CC) $(CFLAGS) -c $< -o $@ $(LIBS)

noteio.o: noteio.c noteio.h salt.h
	$(CC) $(CFLAGS) -c $< -o $@ $(LIBS)

salt.h:
	./gen-salt-header.sh


static-linux64:
	$(CC) $(SRC) /usr/lib/x86_64-linux-gnu/libgcrypt.a /usr/lib/x86_64-linux-gnu/libgpg-error.a -o $(TARGET)

static-win64:
	x86_64-w64-mingw32-gcc $(SRC) /usr/x86_64-w64-mingw32/lib/libgcrypt.a /usr/x86_64-w64-mingw32/lib/libgpg-error.a /usr/x86_64-w64-mingw32/lib/libws2_32.a -o $(TARGET).exe


clean:
	rm -f $(OBJS) $(TARGET) $(TARGET).exe
