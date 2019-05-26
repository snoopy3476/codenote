#include "noteio.h"
#include "passphrase.h"
#include "bytedata.h"

#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>

#include <gcrypt.h>

#define AES_BLOCK_SIZE 16
#define AES_IV_SIZE 12
#define ENCSIZE_SIZE 8
#define BITS_PER_BYTE 8
#define BYTE_FILTER 0xff
#define CNOTE_DEC_CHECK ""


#define ALLOC_LEN (1 << 10)
#define BUF_LEN_SHORT (1 << 7)

#ifndef PASSPHRASE
#define PASSPHRASE ""
#define PASSPHRASE_LEN 0
#endif

#ifndef PASSPHRASE_LEN
#define PASSPHRASE_LEN 0
#endif


#define pad_size(size) ((AES_BLOCK_SIZE - ((size) % AES_BLOCK_SIZE)) % AES_BLOCK_SIZE)
#define min_size(x, y) ( ((x) < (y)) ? (x) : (y) )




size_t encrypt(bytedata * contents, bytedata passphrase);
size_t decrypt(bytedata * contents, bytedata passphrase);


bytedata set_passphrase_postfix(bytedata * passphrase);
byte * encode_data_size(uint64_t size);
uint64_t decode_data_size(byte * size);
bytedata load_file_data(FILE * fp, char const * const delimiters);






size_t read_note(FILE * note, const byte * passphrase, size_t passphrase_len, byte ** data_out)
{
    if (note == NULL || passphrase == NULL || passphrase_len == 0)
	return 0;


    
    // init passphrase object
    bytedata passphrase_bytedata = set_bytedata(NULL, passphrase, passphrase_len);
    if (passphrase_bytedata == NULL)
	return 0;
    
    

    // laod note data and decrypt
    bytedata note_bytedata = load_file_data(note, NULL);
    size_t result_size = decrypt(&note_bytedata, passphrase_bytedata);

    if (result_size == 0 || note_bytedata == NULL)
	return 0;



    // copy the result to data_out
    *data_out = (byte *) malloc(get_bytedata_size(note_bytedata));
    if (*data_out == NULL)
	return 0;

    memcpy(*data_out, get_bytedata(note_bytedata), get_bytedata_size(note_bytedata));
    free_bytedata(note_bytedata);


    
    return result_size;
}

size_t write_note(FILE * note, const byte * passphrase, size_t passphrase_len, const byte * data, size_t data_len)
{
    if (note == NULL || passphrase == NULL || data == NULL || data_len == 0)
	return 0;

    
    
    // init objects
    bytedata passphrase_bytedata = set_bytedata(NULL, passphrase, passphrase_len);
    bytedata note_bytedata = set_bytedata(NULL, data, data_len);
    if (passphrase_bytedata == NULL || note_bytedata == NULL)
	return 0;
    


    // encrypt
    size_t result_size = encrypt(&note_bytedata, passphrase_bytedata);



    // write the result to file pointer
    fwrite(note_bytedata->data, 1, result_size, note);
    free(note_bytedata);

    

    return result_size;
}





size_t encrypt(bytedata * contents, bytedata passphrase)
{
    if (contents == NULL || get_bytedata(*contents) == NULL || get_bytedata_size(*contents) == 0 || get_bytedata(passphrase) == NULL)
	return 0;



    
    // init gcrypt //
    gcry_cipher_hd_t handle;

    gcry_control(GCRYCTL_DISABLE_SECMEM, 0);
    gcry_control(GCRYCTL_INITIALIZATION_FINISHED, 0);
    gcry_cipher_open(&handle, GCRY_CIPHER, GCRY_MODE, 0);


    
    
    // init & set - padding / data size / salt / iv / AAD / key //
    byte padding[AES_BLOCK_SIZE];
    gcry_create_nonce(padding, AES_BLOCK_SIZE);
    
    size_t data_size_orig = get_bytedata_size(*contents);
    
    byte salt[AES_BLOCK_SIZE];
    gcry_create_nonce(salt, AES_BLOCK_SIZE);

    byte iv[AES_BLOCK_SIZE];
    gcry_create_nonce(iv, AES_BLOCK_SIZE);
    
    byte aad[AES_BLOCK_SIZE];
    gcry_create_nonce(aad, AES_BLOCK_SIZE);

    // key
    bytedata passphrase_processed = set_passphrase_postfix(&passphrase);
    if (passphrase_processed == NULL)
	return 0;
    
    size_t key_len = gcry_cipher_get_algo_keylen(GCRY_CIPHER);
    byte * key = (byte *) malloc(sizeof(byte) * key_len);
    gcry_kdf_derive(passphrase_processed, get_bytedata_size(passphrase_processed), GCRY_KDF_PBKDF2, GCRY_CIPHER, salt, AES_BLOCK_SIZE, 16, key_len, key);
    free_bytedata(passphrase_processed);

    

    
    // append & setup & encrypt //
    
    // append padding / data size
    int append_err = 1;
    append_err &= (append_bytedata(contents, padding, pad_size(get_bytedata_size(*contents)) ) ? 1 : 0);
    append_err &= (append_bytedata(contents, encode_data_size(data_size_orig), AES_BLOCK_SIZE) ? 1 : 0);

    // setup
    gcry_cipher_setkey(handle, key, key_len);
    gcry_cipher_setiv(handle, iv, AES_IV_SIZE);
    gcry_cipher_authenticate(handle, aad, AES_BLOCK_SIZE);

    // encrypt
    gcry_cipher_encrypt(handle, get_bytedata(*contents), get_bytedata_size(*contents), NULL, 0);
    free(key);

    // append - salt / iv / ADD / tag
    byte tag[AES_BLOCK_SIZE];
    gcry_cipher_gettag(handle, tag, AES_BLOCK_SIZE);

    append_err &= (append_bytedata(contents, salt, AES_BLOCK_SIZE) ? 1 : 0);
    append_err &= (append_bytedata(contents, iv, AES_BLOCK_SIZE) ? 1 : 0);
    append_err &= (append_bytedata(contents, aad, AES_BLOCK_SIZE) ? 1 : 0);
    append_err &= (append_bytedata(contents, tag, AES_BLOCK_SIZE) ? 1 : 0);


    

    // close gcrypt //
    gcry_cipher_close(handle);


    if (append_err == 0)
	return 0;
    else
	return get_bytedata_size(*contents);
}


size_t decrypt(bytedata * contents, bytedata passphrase)
{
    if (contents == NULL || get_bytedata(*contents) == NULL || get_bytedata(passphrase) == NULL)
	return 0;

    if (get_bytedata_size(*contents) % AES_BLOCK_SIZE != 0)
	return 0;




    // init gcrypt //
    gcry_cipher_hd_t handle;

    gcry_control(GCRYCTL_DISABLE_SECMEM, 0);
    gcry_control(GCRYCTL_INITIALIZATION_FINISHED, 0);
    gcry_cipher_open(&handle, GCRY_CIPHER, GCRY_MODE, 0);



    
    // init, pop & set - tag / AAD / iv / salt / key //
    int pop_err = 1;
    
    byte tag[AES_BLOCK_SIZE];
    pop_err &= (pop_bytedata(contents, tag, AES_BLOCK_SIZE) ? 1 : 0);
    
    byte aad[AES_BLOCK_SIZE];
    pop_err &= (pop_bytedata(contents, aad, AES_BLOCK_SIZE) ? 1 : 0);
    
    byte iv[AES_BLOCK_SIZE];
    pop_err &= (pop_bytedata(contents, iv, AES_BLOCK_SIZE) ? 1 : 0);
    
    byte salt[AES_BLOCK_SIZE];
    pop_err &= (pop_bytedata(contents, salt, AES_BLOCK_SIZE) ? 1 : 0);

    if (pop_err == 0)
	return 0;

    // key
    bytedata passphrase_processed = set_passphrase_postfix(&passphrase);
    if (passphrase_processed == NULL)
	return 0;
    
    size_t key_len = gcry_cipher_get_algo_keylen(GCRY_CIPHER);
    byte * key = (byte *) malloc(sizeof(byte) * key_len);
    gcry_kdf_derive(passphrase_processed, get_bytedata_size(passphrase_processed), GCRY_KDF_PBKDF2, GCRY_CIPHER, salt, AES_BLOCK_SIZE, 16, key_len, key);
    free_bytedata(passphrase_processed);

    


    // setup & decrypt //

    // setup
    gcry_cipher_setkey(handle, key, key_len);
    gcry_cipher_setiv(handle, iv, AES_IV_SIZE);
    gcry_cipher_authenticate(handle, aad, AES_BLOCK_SIZE);

    // decrypt
    gcry_cipher_decrypt(handle, get_bytedata(*contents), get_bytedata_size(*contents), NULL, 0);
    free(key);
    if (gcry_cipher_checktag(handle, tag, AES_BLOCK_SIZE))
	return 0;



    
    // decode original data size //

    // pop data size
    byte data_orig_size_encoded[AES_BLOCK_SIZE];
    pop_bytedata(contents, data_orig_size_encoded, AES_BLOCK_SIZE);

    // decode
    size_t data_orig_size = decode_data_size(data_orig_size_encoded);
    if (data_orig_size == 0)
	return 0;
	
    // pop padding to fit 
    pop_bytedata(contents, NULL, pad_size(data_orig_size));


    
    
    // close gcrypt //
    gcry_cipher_close(handle);

    return data_orig_size;
}






bytedata set_passphrase_postfix(bytedata * passphrase)
{
    if (passphrase == NULL || *passphrase == NULL || get_bytedata(*passphrase) == NULL)
	return NULL;

    bytedata passphrase_processed = append_bytedata(passphrase, (byte *) PASSPHRASE, PASSPHRASE_LEN);
    if (passphrase_processed == NULL)
	return NULL;
    *passphrase = passphrase_processed;
    
    return *passphrase;
}


byte * encode_data_size(uint64_t size)
{
    static byte result[AES_BLOCK_SIZE];
    
    // convert int -> byte (size data)
    for (int i = 0; i < ENCSIZE_SIZE; i++)
	result[i] = (size >> (i*BITS_PER_BYTE)) & BYTE_FILTER;

    return result;
}

uint64_t decode_data_size(byte * size)
{
    if (size == NULL)
	return 0;

    
    uint64_t result = 0;

    // convert byte -> int (size data)
    for (int i = 0; i < ENCSIZE_SIZE; i++)
	result += (size[i] << (i*BITS_PER_BYTE)) & BYTE_FILTER;
    

    return result;
}


bytedata load_file_data(FILE * fp, char const * const delimiters)
{
    if (fp == NULL)
	return NULL;


    
    
    // init
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
    {
	delim_len = 0;
    }


    

    // get input //

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

    


    // init bytedata struct for return using the result
    return set_bytedata(NULL, note_data, input_size);
}
