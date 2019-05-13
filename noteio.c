#include "noteio.h"

#include <stdlib.h>
#include <stdint.h>
#include <gcrypt.h>

#define AES_BLOCK_SIZE 16
#define BYTE_FILTER 0xff

#define encsize(size) ( (size / AES_BLOCK_SIZE + 2) * AES_BLOCK_SIZE )


typedef unsigned char byte;

size_t encrypt(byte ** data, size_t data_size, byte * key_orig, size_t key_orig_len);
size_t decrypt(byte ** data, size_t data_size, byte * key_orig, size_t key_orig_len);

byte * encode_data_size(uint64_t size);
uint64_t decode_data_size(byte * size);

size_t load_file_data(FILE * fp, byte ** out);




size_t read_note(char * file_name, byte * key, size_t key_len)
{
    FILE * note = fopen(file_name, "rb");
    if (note == NULL)
	return 0;
    
    byte * note_data;
    size_t file_size = load_file_data(note, &note_data);
    

    size_t result_size = decrypt(&note_data, file_size, key, key_len);

    
    fclose(note);
    free(note_data);

    return result_size;
}

size_t write_note(char * file_name, byte * key, size_t key_len, byte * data, size_t data_len)
{
    byte * note_data = (byte *) malloc(sizeof(byte) * data_len);
    memcpy(note_data, data, data_len);
    
    FILE * note = fopen(file_name, "wb");
    if (note == NULL)
	return 0;

    
    size_t result_size = encrypt(&note_data, data_len, key, key_len);
    fwrite(note_data, 1, result_size, note);
    

    fclose(note);
    free(note_data);

    return result_size;
}






size_t encrypt(byte ** data, size_t data_size, byte * key_orig, size_t key_orig_len)
{
    size_t encdata_size = encsize(data_size);
    *data = (byte *) realloc(*data, sizeof(byte) * encdata_size);
    memcpy(*data + encdata_size - AES_BLOCK_SIZE,
	   encode_data_size(data_size),
	   sizeof(byte) * AES_BLOCK_SIZE);
    //byte * data_size_data = encode_data_size(data_size);
    
    size_t key_len = gcry_cipher_get_algo_keylen(GCRY_CIPHER);

    byte * key_final = (byte *) malloc(sizeof(byte) * key_len);


    gcry_cipher_hd_t handle;

    gcry_control(GCRYCTL_DISABLE_SECMEM, 0);
    gcry_control(GCRYCTL_INITIALIZATION_FINISHED, 0);
    gcry_cipher_open(&handle, GCRY_CIPHER, GCRY_MODE, 0);

    gcry_kdf_derive(key_orig, key_orig_len, GCRY_KDF_PBKDF2, GCRY_CIPHER, KEY_SALT, strnlen(KEY_SALT, 1024), 16, key_len, key_final);
    gcry_cipher_setkey(handle, key_final, key_len);

    gcry_cipher_encrypt(handle, *data, encdata_size, NULL, 0);

    /*
    size_t index;
    for (index = 0; index < data_size; index++)
        printf("%c", (*data)[index]);
    printf("\n");
    */

    gcry_cipher_close(handle);
    
    free(key_final);

    return encdata_size;
}


size_t decrypt(byte ** data, size_t data_size, byte * key_orig, size_t key_orig_len)
{
    size_t key_len = gcry_cipher_get_algo_keylen(GCRY_CIPHER);

    byte * key_final = (byte *) malloc(sizeof(byte) * key_len);


    gcry_cipher_hd_t handle;

    gcry_control(GCRYCTL_DISABLE_SECMEM, 0);
    gcry_control(GCRYCTL_INITIALIZATION_FINISHED, 0);
    gcry_cipher_open(&handle, GCRY_CIPHER, GCRY_MODE, 0);

    gcry_kdf_derive(key_orig, key_orig_len, GCRY_KDF_PBKDF2, GCRY_CIPHER, KEY_SALT, strnlen(KEY_SALT, 1024), 16, key_len, key_final);
    gcry_cipher_setkey(handle, key_final, key_len);

    gcry_cipher_decrypt(handle, *data, data_size, NULL, 0);

    //data = (byte *) realloc(data, sizeof(byte) * encdata_size);
    size_t origdata_size = decode_data_size(*data + data_size - AES_BLOCK_SIZE);
    *data = (byte *) realloc(*data, sizeof(byte) * (origdata_size + 1));
    (*data)[origdata_size] = '\0';
    

    size_t index;
    for (index = 0; index < origdata_size; index++)
        printf("%c", (*data)[index]);
    printf("\n");

    gcry_cipher_close(handle);
    
    free(key_final);

    return origdata_size;
}







byte * encode_data_size(uint64_t size)
{
    static byte result[8];

    for (int i = 0; i < 8; i++)
	result[i] = (size >> (i*8)) & BYTE_FILTER;

    return result;
}

uint64_t decode_data_size(byte * size)
{
    uint64_t result = 0;
    for (int i = 0; i < 8; i++)
	result += (size[i] << (i*8)) & BYTE_FILTER;

    return result;
}



size_t load_file_data(FILE * fp, byte ** out)
{
    byte * note_data = (byte *) malloc(sizeof(byte) * ALLOC_LEN);
    size_t file_size = 0, alloc_step = 0;

    while (!feof(fp))
    {
	if (file_size > sizeof(byte) * alloc_step * ALLOC_LEN)
	    note_data = realloc(note_data, sizeof(byte) * ALLOC_LEN * (++alloc_step + 1));
	file_size += fread(note_data + file_size, sizeof(byte), ALLOC_LEN, fp);
    }

    *out = realloc(note_data, sizeof(byte) * file_size);

    return file_size;
}
