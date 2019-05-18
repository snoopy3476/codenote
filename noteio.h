#ifndef __CODENOTE_NOTEIO_H__
#define __CODENOTE_NOTEIO_H__
#include <stdio.h>

#define ALLOC_LEN 1 << 20


#define GCRY_CIPHER GCRY_CIPHER_AES256
#define GCRY_MODE GCRY_CIPHER_MODE_ECB

size_t read_note(FILE * note, unsigned char * key, size_t key_len);
size_t write_note(FILE * note, unsigned char * key, size_t key_len, unsigned char * data, size_t data_len);


#endif
