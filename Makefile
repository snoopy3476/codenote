CC := gcc
MAKE := make
override CFLAGS += -Wall

TARGET := cnote
SRC := codenote.c noteio.c
OBJS := $(SRC:.c=.o)
LIBS := -l:libgcrypt.a -l:libgpg-error.a
LIBS_D := -lgcrypt
HEADERS_CUSTOMIZABLE := salt.h theme.h


TARGET_W := cnote.exe
OBJS_W := $(OBJS:.o=.wo)
LIBS_W := $(LIBS) -l:libws2_32.a

LIBS_CYG := $(LIBS:.a=.dll.a)



all: $(TARGET)

dynamic: $(TARGET).dynamic



$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $^ -o $@ $(LIBS)

$(TARGET).dynamic: $(OBJS)
	$(CC) $(CFLAGS) $^ -o $@ $(LIBS_D)



codenote.%o: codenote.c noteio.h ansiseq.h ansicolor.h theme.h
	$(CC) $(CFLAGS) -c $< -o $@

noteio.%o: noteio.c noteio.h salt.h
	$(CC) $(CFLAGS) -c $< -o $@

codenote.c: theme.h
noteio.c: salt.h


$(HEADERS_CUSTOMIZABLE):
	cp -af $(@:.h=-default.h) $@



# Packages 'gcc-mingw-w64-x86-64', 'libgcrypt-mingw-w64-dev' are required
win:
	$(MAKE) \
		CC="x86_64-w64-mingw32-gcc" \
		LIBS="$(LIBS_W)" \
		OBJS="$(OBJS_W)" \
		TARGET="$(TARGET_W)"

# For Cygwin gcc in Windows
cyg:
	$(MAKE) LIBS="$(LIBS_CYG)"



clean:
	rm -f $(OBJS) $(OBJS_W) $(TARGET) $(TARGET).dynamic $(TARGET_W)
