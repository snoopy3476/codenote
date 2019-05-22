#ifndef __CODENOTE_NOTEIO_H__
#define __CODENOTE_NOTEIO_H__
#include <stdio.h>


#define GCRY_CIPHER GCRY_CIPHER_AES256
#define GCRY_MODE GCRY_CIPHER_MODE_GCM


typedef unsigned char byte;

size_t read_note(FILE * note, unsigned char * key, size_t key_len, byte ** data_out);
size_t write_note(FILE * note, unsigned char * key, size_t key_len, byte * data, size_t data_len);
size_t load_file_data(FILE * fp, byte ** out, char const * const delimiters);


#endif
