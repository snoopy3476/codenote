###
### If you failed to link with libgcrypt in default run,
### make again with followings:
###
### $ make GCRY_PATH=/path/to/gcrypt/library
###

MAKE := make


# go to src then make
all:
	$(MAKE) $@ -C src
%:
	$(MAKE) $@ -C src
