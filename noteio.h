#ifndef __CODENOTE_NOTEIO_H__
#define __CODENOTE_NOTEIO_H__
#include <stdio.h>

#define ALLOC_LEN (1 << 10)


#define GCRY_CIPHER GCRY_CIPHER_AES256
#define GCRY_MODE GCRY_CIPHER_MODE_ECB


typedef unsigned char byte;

size_t read_note(FILE * note, unsigned char * key, size_t key_len, byte ** data_out);
size_t write_note(FILE * note, unsigned char * key, size_t key_len, byte * data, size_t data_len);

void rand_seed();


#endif
