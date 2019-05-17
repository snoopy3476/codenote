CC := gcc
MAKE := make
override CFLAGS += -Wall

TARGET := codenote
SRC := codenote.c noteio.c
OBJS := $(SRC:.c=.o)
LIBS := -l:libgcrypt.a -l:libgpg-error.a
LIBS_D := -lgcrypt



all: $(TARGET)

dynamic: $(TARGET).dynamic



$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $^ -o $@ $(LIBS)

$(TARGET).dynamic: $(OBJS)
	$(CC) $(CFLAGS) $^ -o $@ $(LIBS_D)



codenote.o: codenote.c noteio.h ansiseq.h
	$(CC) $(CFLAGS) -c $< -o $@

noteio.o: noteio.c noteio.h salt.h
	$(CC) $(CFLAGS) -c $< -o $@



salt.h:
	cp -af salt-default.h salt.h



static-win64:
	x86_64-w64-mingw32-gcc $(SRC) /usr/x86_64-w64-mingw32/lib/libgcrypt.a /usr/x86_64-w64-mingw32/lib/libgpg-error.a /usr/x86_64-w64-mingw32/lib/libws2_32.a -o $(TARGET).exe



clean:
	rm -f $(OBJS) $(TARGET) $(TARGET).dynamic $(TARGET).exe
