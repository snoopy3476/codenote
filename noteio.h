#ifndef __CODENOTE_NOTEIO_H__
#define __CODENOTE_NOTEIO_H__
#include <stdio.h>


#define GCRY_CIPHER GCRY_CIPHER_AES256
#define GCRY_MODE GCRY_CIPHER_MODE_GCM


size_t read_note(FILE * note, const unsigned char * passphrase, size_t passphrase_len, unsigned char ** data_out);
size_t write_note(FILE * note, const unsigned char * passphrase, size_t passphrase_len, const unsigned char * data, size_t data_len);

#endif
