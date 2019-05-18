#include "noteio.h"
#include "salt.h"

#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <gcrypt.h>

#define AES_BLOCK_SIZE 16
#define ENCSIZE_SIZE 8
#define BITS_PER_BYTE 8
#define BYTE_FILTER 0xff
#define CNOTE_DEC_CHECK ""

#define encsize(size) ( ((size) / AES_BLOCK_SIZE + 2) * AES_BLOCK_SIZE )
#define min_size(x, y) ( ((x) < (y)) ? (x) : (y) )


typedef unsigned char byte;

size_t encrypt(byte ** data, size_t data_size, byte * key_orig, size_t key_orig_len);
size_t decrypt(byte ** data, size_t data_size, byte * key_orig, size_t key_orig_len);

byte * encrypt_data_size(uint64_t size, byte * key, size_t key_len);
uint64_t decrypt_data_size(byte * size, byte * key, size_t key_len);

size_t load_file_data(FILE * fp, byte ** out);




size_t read_note(FILE * note, byte * key, size_t key_len)
{
    if (note == NULL || key == NULL)
	return 0;

    
    byte * note_data;
    size_t file_size = load_file_data(note, &note_data);
    

    size_t result_size = decrypt(&note_data, file_size, key, key_len);

    
    free(note_data);

    return result_size;
}

size_t write_note(FILE * note, byte * key, size_t key_len, byte * data, size_t data_len)
{
    if (note == NULL || key == NULL || data == NULL || data_len == 0)
	return 0;

    
    byte * note_data = (byte *) malloc(sizeof(byte) * data_len);
    memcpy(note_data, data, data_len);

    
    size_t result_size = encrypt(&note_data, data_len, key, key_len);
    fwrite(note_data, 1, result_size, note);
    

    free(note_data);

    return result_size;
}






size_t encrypt(byte ** data, size_t data_size, byte * key_orig, size_t key_orig_len)
{
    if (data == NULL || *data == NULL)
	return 0;

    
    
    size_t key_len = gcry_cipher_get_algo_keylen(GCRY_CIPHER);
    byte * key_final = (byte *) malloc(sizeof(byte) * key_len);


    gcry_cipher_hd_t handle;

    gcry_control(GCRYCTL_DISABLE_SECMEM, 0);
    gcry_control(GCRYCTL_INITIALIZATION_FINISHED, 0);
    gcry_cipher_open(&handle, GCRY_CIPHER, GCRY_MODE, 0);

    gcry_kdf_derive(key_orig, key_orig_len, GCRY_KDF_PBKDF2, GCRY_CIPHER, KEY_SALT, KEY_SALT_LEN, 16, key_len, key_final);
    gcry_cipher_setkey(handle, key_final, key_len);

    
    size_t encdata_size = encsize(data_size);
    *data = (byte *) realloc(*data, sizeof(byte) * encdata_size);
    memcpy(*data + encdata_size - AES_BLOCK_SIZE,
	   encrypt_data_size(data_size, key_final, key_len),
	   sizeof(byte) * AES_BLOCK_SIZE);

    
    gcry_cipher_encrypt(handle, *data, encdata_size, NULL, 0);

    gcry_cipher_close(handle);
    
    free(key_final);

    return encdata_size;
}


size_t decrypt(byte ** data, size_t data_size, byte * key_orig, size_t key_orig_len)
{
    if (data == NULL || *data == NULL || data_size < AES_BLOCK_SIZE)
	return 0;

    
    size_t key_len = gcry_cipher_get_algo_keylen(GCRY_CIPHER);

    byte * key_final = (byte *) malloc(sizeof(byte) * key_len);


    gcry_cipher_hd_t handle;

    gcry_control(GCRYCTL_DISABLE_SECMEM, 0);
    gcry_control(GCRYCTL_INITIALIZATION_FINISHED, 0);
    gcry_cipher_open(&handle, GCRY_CIPHER, GCRY_MODE, 0);

    gcry_kdf_derive(key_orig, key_orig_len, GCRY_KDF_PBKDF2, GCRY_CIPHER, KEY_SALT, KEY_SALT_LEN, 16, key_len, key_final);
    gcry_cipher_setkey(handle, key_final, key_len);

    gcry_cipher_decrypt(handle, *data, data_size, NULL, 0);

    size_t origdata_size = decrypt_data_size(*data + data_size - AES_BLOCK_SIZE, key_final, key_len);

    // check if decoded size is valid
    if (origdata_size == 0)
	return 0;
    
    *data = (byte *) realloc(*data, sizeof(byte) * (origdata_size + 1));
    (*data)[origdata_size] = '\0';
    

    size_t index;
    for (index = 0; index < origdata_size; index++)
        printf("%c", (*data)[index]);

    gcry_cipher_close(handle);
    
    free(key_final);

    return origdata_size;
}







byte * encrypt_data_size(uint64_t size, byte * key, size_t key_len)
{
    static byte result[AES_BLOCK_SIZE];
    
    // convert int -> byte (size data)
    for (int i = 0; i < ENCSIZE_SIZE; i++)
	result[i] = (size >> (i*ENCSIZE_SIZE)) & BYTE_FILTER;

    // append check value (for decryption)
    memcpy(&result[ENCSIZE_SIZE], key, min_size(AES_BLOCK_SIZE - ENCSIZE_SIZE, key_len));

    return result;
}

uint64_t decrypt_data_size(byte * size, byte * key, size_t key_len)
{
    if (size == NULL)
	return 0;

    
    uint64_t result = 0;
    byte check[AES_BLOCK_SIZE - ENCSIZE_SIZE];

    // convert byte -> int (size data)
    for (int i = 0; i < ENCSIZE_SIZE; i++)
	result += (size[i] << (i*BITS_PER_BYTE)) & BYTE_FILTER;

    // check if valid decryption
    size_t check_size = min_size(AES_BLOCK_SIZE - ENCSIZE_SIZE, key_len);
    memcpy(check, &size[ENCSIZE_SIZE], check_size);
    if (memcmp(check, key, check_size) != 0)
	return 0;
    

    return result;
}



size_t load_file_data(FILE * fp, byte ** out)
{
    if (fp == NULL)
    {
	*out = NULL;
	return 0;
    }

    
    byte * note_data = (byte *) malloc(sizeof(byte) * ALLOC_LEN);
    size_t file_size = 0, alloc_step = 0;

    while (!feof(fp))
    {
	note_data = realloc(note_data, sizeof(byte) * ALLOC_LEN * (++alloc_step + 1));
	file_size += fread(note_data + file_size, sizeof(byte), ALLOC_LEN, fp);
    }

    if (file_size != 0)
	*out = realloc(note_data, sizeof(byte) * file_size);
    else
	*out = NULL;

    return file_size;
}
