#include "noteio.h"
#include "passphrase.h"

#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>

#include "gcrypt.h"

#define AES_BLOCK_SIZE 16
#define AES_SALT_SIZE 16
#define AES_IV_SIZE 12
#define ENCSIZE_SIZE 8
#define BITS_PER_BYTE 8
#define BYTE_FILTER 0xff
#define CNOTE_DEC_CHECK ""


#define ALLOC_LEN (1 << 10)
#define BUF_LEN_SHORT (1 << 7)

#define encsize(size) ( ((size - 1) / AES_BLOCK_SIZE + 6) * AES_BLOCK_SIZE )
#define min_size(x, y) ( ((x) < (y)) ? (x) : (y) )



size_t encrypt(byte ** data, size_t data_size, byte * passphrase, size_t passphrase_len);
size_t decrypt(byte ** data, size_t data_size, byte * passphrase, size_t passphrase_len);


byte * set_passphrase_postfix(byte * passphrase, size_t passphrase_len);
byte * encrypt_data_size(uint64_t size);
uint64_t decrypt_data_size(byte * size);





size_t read_note(FILE * note, byte * passphrase, size_t passphrase_len, byte ** data_out)
{
    if (note == NULL || passphrase == NULL)
	return 0;

    
    byte * note_data;
    size_t file_size = load_file_data(note, &note_data, NULL);
    

    size_t result_size = decrypt(&note_data, file_size, passphrase, passphrase_len);



    // pass data to data_out ptr if data_out != NULL
    if (result_size != 0 && data_out != NULL)
    {
	*data_out = note_data;
    }

    // print data if data_out == NULL
    else
    {
	for (size_t index = 0; index < result_size; index++)
	    printf("%c", note_data[index]);
	
	free(note_data);
    }

    return result_size;
}

size_t write_note(FILE * note, byte * passphrase, size_t passphrase_len, byte * data, size_t data_len)
{
    if (note == NULL || passphrase == NULL || data == NULL || data_len == 0)
	return 0;

    
    byte * note_data = (byte *) malloc(sizeof(byte) * data_len);
    memcpy(note_data, data, data_len);

    
    size_t result_size = encrypt(&note_data, data_len, passphrase, passphrase_len);
    fwrite(note_data, 1, result_size, note);
    

    free(note_data);

    return result_size;
}






size_t encrypt(byte ** data, size_t data_size, byte * passphrase, size_t passphrase_len)
{
    if (data == NULL || *data == NULL || passphrase == NULL)
	return 0;




    // preprocess passphrase with postfix
    byte * passphrase_processed = set_passphrase_postfix(passphrase, passphrase_len);
    if (passphrase_processed == NULL)
	return 0;

    

    

    // init gcrypt //
    gcry_cipher_hd_t handle;

    gcry_control(GCRYCTL_DISABLE_SECMEM, 0);
    gcry_control(GCRYCTL_INITIALIZATION_FINISHED, 0);
    gcry_cipher_open(&handle, GCRY_CIPHER, GCRY_MODE, 0);

    
    
    // append data size info //
    size_t encdata_size = encsize(data_size);
    byte * data_realloc = (byte *) realloc(*data, sizeof(byte) * encdata_size);
    if (data_realloc == NULL)
    {
	return 0;
    }
    else
    {
	*data = data_realloc;

	byte * encdata_size_ptr = *data + encdata_size - 5*AES_BLOCK_SIZE;
	memcpy(encdata_size_ptr,
	       encrypt_data_size(data_size),
	       sizeof(byte) * AES_BLOCK_SIZE);
    }

    
    // init key, salt //
    byte * salt_ptr = *data + encdata_size - 3*AES_BLOCK_SIZE;
    gcry_create_nonce(salt_ptr, AES_SALT_SIZE);
    size_t key_len = gcry_cipher_get_algo_keylen(GCRY_CIPHER);
    byte * key = (byte *) malloc(sizeof(byte) * key_len);
    gcry_kdf_derive(passphrase_processed, passphrase_len + PASSPHRASE_LEN, GCRY_KDF_PBKDF2, GCRY_CIPHER, salt_ptr, AES_SALT_SIZE, 16, key_len, key);
    gcry_cipher_setkey(handle, key, key_len);
    free(passphrase_processed);


    // init iv , AAD //
    byte * iv_ptr = *data + encdata_size - 4*AES_BLOCK_SIZE;
    gcry_create_nonce(iv_ptr, AES_BLOCK_SIZE);
    gcry_cipher_setiv(handle, iv_ptr, AES_IV_SIZE);
    byte * auth_ptr = *data + encdata_size - 2*AES_BLOCK_SIZE;
    gcry_create_nonce(auth_ptr, AES_BLOCK_SIZE);
    gcry_cipher_authenticate(handle, auth_ptr, AES_BLOCK_SIZE);


    // encrypt //
    gcry_cipher_encrypt(handle, *data, encdata_size - 4*AES_BLOCK_SIZE, NULL, 0);
    gcry_cipher_gettag(handle, *data + encdata_size - AES_BLOCK_SIZE, AES_BLOCK_SIZE);


    // close gcrypt //
    gcry_cipher_close(handle);
    free(key);

    return encdata_size;
}


size_t decrypt(byte ** data, size_t data_size, byte * passphrase, size_t passphrase_len)
{
    if (data == NULL || *data == NULL || passphrase == NULL || data_size < AES_BLOCK_SIZE)
	return 0;




    // preprocess passphrase with postfix
    byte * passphrase_processed = set_passphrase_postfix(passphrase, passphrase_len);
    if (passphrase_processed == NULL)
	return 0;
    

    

    // init gcrypt //
    gcry_cipher_hd_t handle;

    gcry_control(GCRYCTL_DISABLE_SECMEM, 0);
    gcry_control(GCRYCTL_INITIALIZATION_FINISHED, 0);
    gcry_cipher_open(&handle, GCRY_CIPHER, GCRY_MODE, 0);


    // init key //
    byte * salt_ptr = *data + data_size - 3*AES_BLOCK_SIZE;
    size_t key_len = gcry_cipher_get_algo_keylen(GCRY_CIPHER);
    byte * key = (byte *) malloc(sizeof(byte) * key_len);
    gcry_kdf_derive(passphrase_processed, passphrase_len + PASSPHRASE_LEN, GCRY_KDF_PBKDF2, GCRY_CIPHER, salt_ptr, AES_SALT_SIZE, 16, key_len, key);
    gcry_cipher_setkey(handle, key, key_len);
    free(passphrase_processed);


    // init iv //
    gcry_cipher_setiv(handle, *data + data_size - 4*AES_BLOCK_SIZE, AES_IV_SIZE);
    gcry_cipher_authenticate(handle, *data + data_size - 2*AES_BLOCK_SIZE, AES_BLOCK_SIZE);


    // decrypt //
    gcry_cipher_decrypt(handle, *data, data_size - 4*AES_BLOCK_SIZE, NULL, 0);
    free(key);
    if (gcry_cipher_checktag(handle, *data + data_size - AES_BLOCK_SIZE, AES_BLOCK_SIZE))
	return 0;


    // get original data size //
    size_t origdata_size = decrypt_data_size(*data + data_size - 5*AES_BLOCK_SIZE);
    // check if decrypted size is valid
    if (origdata_size == 0)
	return 0;
	

    // change allocated size to fit //
    byte * data_realloc = (byte *) realloc(*data, sizeof(byte) * (origdata_size + 1));
    if (data_realloc == NULL)
	return 0;
    else
	*data = data_realloc;
    
    // Set '\0' after the last byte of the data
    // to make printf easier if data are string
    (*data)[origdata_size] = '\0';


    
    // close gcrypt //
    gcry_cipher_close(handle);

    return origdata_size;
}






byte * set_passphrase_postfix(byte * passphrase, size_t passphrase_len)
{
    if (passphrase == NULL)
	return NULL;
    
    byte * passphrase_processed = (byte *) malloc(sizeof(byte) * (passphrase_len + PASSPHRASE_LEN));
    if (passphrase_processed == NULL)
	return NULL;

    
    memcpy(passphrase_processed, passphrase, passphrase_len);
    memcpy(passphrase_processed + passphrase_len, PASSPHRASE, PASSPHRASE_LEN);
    
    return passphrase_processed;
}


byte * encrypt_data_size(uint64_t size)
{
    static byte result[AES_BLOCK_SIZE];
    
    // convert int -> byte (size data)
    for (int i = 0; i < ENCSIZE_SIZE; i++)
	result[i] = (size >> (i*BITS_PER_BYTE)) & BYTE_FILTER;

    return result;
}

uint64_t decrypt_data_size(byte * size)
{
    if (size == NULL)
	return 0;

    
    uint64_t result = 0;

    // convert byte -> int (size data)
    for (int i = 0; i < ENCSIZE_SIZE; i++)
	result += (size[i] << (i*BITS_PER_BYTE)) & BYTE_FILTER;
    

    return result;
}


size_t load_file_data(FILE * fp, byte ** out, char const * const delimiters)
{
    if (out == NULL)
	return 0;
    
    if (fp == NULL)
    {
	*out = NULL;
	return 0;
    }

    byte * note_data = (byte *) malloc(sizeof(byte) * ALLOC_LEN),
	* note_data_realloc = NULL;
    size_t input_size = 0, alloc_step = 0;


    
    // set delimiter length
    
    size_t delim_len;
    if (delimiters != NULL)
    {
	if (delimiters[0] == '\0')
	    delim_len = strnlen(delimiters + 1, BUF_LEN_SHORT) + 1;
	else
	    delim_len = strnlen(delimiters, BUF_LEN_SHORT);
    }
    else
	delim_len = 0;

    

    // get input


    // if stdin input
    if (fp == stdin)
    {
	fgets((char *)note_data + input_size, sizeof(byte) * ALLOC_LEN, fp);
	    
	input_size = strnlen((char *)note_data, ALLOC_LEN);
	
	char * last_element = (char *) &note_data[input_size - 1];
	if (*last_element == '\n')
	{
	    *last_element = '\0';
	    input_size--;
	}
    }

    // if file input
    else
    {
	// read_size[0]: length of byte after delim cut
	// read_size[1]: length of orig fread
	size_t read_size[2] = {0, 0};
    
	while ( !feof(fp) && (read_size[0] == read_size[1]) )
	{
	    read_size[0] = fread(note_data + input_size, sizeof(byte), ALLOC_LEN, fp);
	    read_size[1] = read_size[0];

	    // cut input if one of delimiter exists at input
	    if (delimiters != NULL)
		for (size_t i = 0; i < read_size[0]; i++)
		    for (size_t delim_i = 0; delim_i < delim_len; delim_i++)
			if (note_data[input_size + i] == delimiters[delim_i])
			    read_size[0] = i;
			

	
	    note_data_realloc = realloc(note_data, sizeof(byte) * ALLOC_LEN * (++alloc_step + 1));
	    if (note_data_realloc == NULL && note_data != NULL)
	    {
		free(note_data);
		break;
	    }

	    note_data = note_data_realloc;
	    input_size += read_size[0];
	}
    }



    if (out != NULL)
    {
	*out = NULL;
	
	if (input_size != 0 && note_data != NULL)
	    *out = realloc(note_data, sizeof(byte) * input_size + 1);
	
	if (*out == NULL)
	    free(note_data);
	else
	    (*out)[input_size] = '\0'; // NULL termination for string input
    }
    else
    {
	free(note_data);
    }

    

    return input_size;
}
