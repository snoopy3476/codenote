#ifndef __CODENOTE_NOTEIO_H__
#define __CODENOTE_NOTEIO_H__
#include <stdio.h>

#define ALLOC_LEN 1 << 20


#define GCRY_CIPHER GCRY_CIPHER_AES256
#define GCRY_MODE GCRY_CIPHER_MODE_ECB
#define KEY_SALT "ASDFwasdivboadvc7y9263 r21 2`vcfc9x8!#$^4trh0 8y24351-0379gc da @!#%&%*U&^I^&JYTBG 135b af ;[q\1241$!~~@#%3dsgavc"


size_t read_note(char * file_name, unsigned char * key, size_t key_len);
size_t write_note(char * file_name, unsigned char * key, size_t key_len, unsigned char * data, size_t data_len);


#endif
