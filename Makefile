CC := gcc
MAKE := make
override CFLAGS += -Wall

TARGET := cnote
SRC := codenote.c noteio.c bytedata.c
OBJS := $(SRC:.c=.o)
LIBS := -l:libgcrypt.a -l:libgpg-error.a
LIBS_D := -lgcrypt
HEADERS_CUSTOMIZABLE := passphrase.h theme.h


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


%.o %.wo: %.c
	$(CC) $(CFLAGS) -c $< -o $@

codenote.o codenote.wo: noteio.h ansiseq.h ansicolor.h theme.h
noteio.o noteio.wo: noteio.h bytedata.h passphrase.h
bytedata.o bytedata.wo: bytedata.h



$(HEADERS_CUSTOMIZABLE):
	cp -af $(@:.h=-default.h) $@



# Packages 'gcc-mingw-w64-x86-64', 'libgcrypt-mingw-w64-dev' are required
win:
	$(MAKE) --no-print-directory \
		CC="x86_64-w64-mingw32-gcc" \
		LIBS="$(LIBS_W)" \
		OBJS="$(OBJS_W)" \
		TARGET="$(TARGET_W)"

# For Cygwin gcc in Windows
cyg:
	$(MAKE) --no-print-directory \
		LIBS="$(LIBS_CYG)"



clean:
	rm -f $(OBJS) $(OBJS_W) $(TARGET) $(TARGET).dynamic $(TARGET_W)
