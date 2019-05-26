###
### If you failed to link with libgcrypt in default run,
### make again with followings:
###
### $ make GCRY_PATH=/path/to/gcrypt/library
###





### VARIABLES ###

# binaries
MAKE := make
CC := $(BIN_PREFIX)gcc
AR := $(BIN_PREFIX)ar
LD := $(BIN_PREFIX)ld
override CFLAGS += -Wall

# dir
BIN_DIR := ../bin
LIB_DIR := ../lib
BYTEDATA := bytedata
THEME := theme
DIR_REQUIRED := $(BIN_DIR) $(LIB_DIR)

# extensions
EXT_C := c
EXT_H := h
EXT_O := o
EXT_O_W := win.o
EXT_A := a
EXT_A_W := win.a

# targets
TARGET_NAME := cnote
TARGET := $(BIN_DIR)/$(TARGET_NAME)
TARGET_LIB := $(LIB_DIR)/cnote.$(EXT_A)
TARGET_LIB_OBJ := $(TARGET_NAME:=.$(EXT_O))
TARGET_D := $(BIN_DIR)/$(TARGET_NAME)-dynamic

# sources
SRCS := noteio.$(EXT_C) $(BYTEDATA)/bytedata.$(EXT_C)
SRC_MAIN := codenote.$(EXT_C)

# objects
OBJS := $(SRCS:.$(EXT_C)=.$(EXT_O))
OBJ_MAIN := $(SRC_MAIN:.$(EXT_C)=.$(EXT_O))

# libraries
LIBS_D := -lgcrypt
LIBS := $(LIBS_D) -lgpg-error
LIBS_W := $(LIBS) -lws2_32

GCRY_PATH := /usr/lib/x86_64-linux-gnu/
CGRY_LIB_OPT := $(addprefix -L, $(GCRY_PATH))

# headers
HEADERS_CUSTOMIZABLE := passphrase.$(EXT_H) $(THEME)/theme.$(EXT_H)





### RULES ###

all: $(TARGET)

# cnote binary
$(TARGET): $(OBJ_MAIN) $(TARGET_LIB)
	$(CC) $(CFLAGS) $(OBJ_MAIN) -o $@ $(TARGET_LIB)


# cnote static library
$(TARGET_LIB): $(TARGET_LIB_OBJ)
	$(MAKE) --no-print-directory  $(DIR_REQUIRED)
	$(AR) rcs $@ $^

# cnote static library object
$(TARGET_LIB_OBJ): $(OBJS)
	$(LD) -static -r -o $@ $^ $(CGRY_LIB_OPT) $(LIBS)


# general object files
%.win.o %.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# object file dependencies
codenote.win.o codenote.o: noteio.h $(THEME)/ansiseq.h $(THEME)/ansicolor.h $(THEME)/theme.h
noteio.win.o noteio.o: noteio.h $(BYTEDATA)/bytedata.h passphrase.h


# get new headers from default headers if not exist
$(HEADERS_CUSTOMIZABLE):
	cp -af $(@:.h=-default.h) $@

# create new directories if not exist
$(DIR_REQUIRED):
	mkdir -p $@


# clean all
clean:
	$(MAKE) --no-print-directory clean-linux
	$(MAKE) --no-print-directory clean-win

# clean linux
clean-linux:
	rm -f $(OBJS) $(OBJS_W) $(OBJ_MAIN) $(OBJ_MAIN_W) $(TARGET_LIB) $(TARGET_LIB_W) $(TARGET_LIB_OBJ) $(TARGET_LIB_OBJ_W) $(TARGET) $(TARGET_D) $(TARGET_W)





### OTHER VERSIONS ###

# Link dynamically with gcrypt
dynamic: $(TARGET_D)

$(TARGET_D): $(OBJ_MAIN) $(OBJS)
	$(MAKE) --no-print-directory  $(DIR_REQUIRED)
	$(CC) $(CFLAGS) $^ -o $@ $(LIBS_D)


# Windows binary
# For mingw gcc
# Packages 'gcc-mingw-w64-x86-64', 'libgcrypt-mingw-w64-dev' are required
win: GCRY_PATH=/usr/x86_64-w64-mingw32/lib/
win:
	$(MAKE) --no-print-directory \
		BIN_PREFIX="x86_64-w64-mingw32-" \
		TARGET="$(TARGET:=.exe)" \
		GCRY_PATH="$(GCRY_PATH)" \
		LIBS="$(LIBS_W)" \
		EXT_A="$(EXT_A_W)" \
		EXT_O="$(EXT_O_W)"

# clean windows
clean-win:
	$(MAKE) clean-linux --no-print-directory \
		TARGET="$(TARGET:=.exe)" \
		EXT_A="$(EXT_A_W)" \
		EXT_O="$(EXT_O_W)"
