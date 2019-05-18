#include "noteio.h"
#include "ansiseq.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BUF_LEN 1024
#define KEY_BUF_LEN 1 << 15
#define DATA_BUF_LEN 1 << 20

#define EXT ".cnote"
#define EXT_LEN 6

#define HKS_BASE "                                                                                                    "
#define HIDE_KEY_STR HKS_BASE HKS_BASE HKS_BASE HKS_BASE HKS_BASE HKS_BASE HKS_BASE HKS_BASE HKS_BASE HKS_BASE


#define new_page() printf(MOVE_CURSOR NEWPAGE, 1, 1)
#define clear(offset_x, offset_y) printf(MOVE_CURSOR CLEAR, (offset_y)+1, (offset_x)+1)


typedef unsigned char byte;

typedef enum workmode_t {WM_NONE, WM_ENCRYPT, WM_DECRYPT} workmode_t;
typedef enum fopenmode_t {FM_READ = 0, FM_WRITE} fopenmode_t;
char const * const FOPENMODE_STR[2] =
{
    "rb",
    "wb+"
};

size_t get_input(const char * prompt, byte * buf_out, size_t buf_len, int is_retype);
size_t get_stdin(char * buf, size_t buf_len);

FILE * get_file(char ** file_name, fopenmode_t fopenmode);
byte * get_key(byte * key, size_t * key_len_out, int is_retype);
byte * get_data(byte * data, size_t * data_len_out);

void print_title(char * title);


int main(int argc, char* argv[])
{
    char * file_name = NULL;
    byte * key = NULL;
    size_t key_len;
    byte * data = NULL;
    size_t data_len;
    FILE * note = NULL;
    int interactive = 1;



	
    workmode_t workmode = WM_NONE;
    if (argc > 1 && strncmp(argv[1], "-e", BUF_LEN) == 0)
    {
	workmode = WM_ENCRYPT;

	switch (argc)
	{
	default:
	    interactive = 0; // Nothing to get input from stdin
	    
	    data = (byte *) argv[4];
	case 4:
	    key = (byte *) argv[3];
	case 3:
	    file_name = argv[2];

	    break;
	case 2:
	case 1:
	case 0:
	    break;
	}
    }
    else
    {
	workmode = WM_DECRYPT;

	switch (argc)
	{
	default:
	    interactive = 0; // Nothing to get input from stdin
	    
	    key = (byte *) argv[2];
	case 2:
	    file_name = argv[1];

	    break;
	case 1:
	case 0:
	    break;
	}
    }

	



    // encrypt || decrypt

    switch (workmode)
    {
	
	// encrypt mode
    case WM_ENCRYPT:
    {
	// name input
	note = get_file(&file_name, FM_WRITE);

	// print title
	if (interactive)
	{
	    clear(0, 0);
	    print_title(file_name);
	}
    
	// key input
	key = get_key(key, &key_len, 1);

	// data input
	data = get_data(data, &data_len);


	
	size_t write_size = write_note(note, key, key_len, data, data_len);
	
	if (write_size == 0)
	    printf("\t" COLOR(FG_BLACK BG_RED) " *** ENCRYPT ERROR *** " COLOR(FG_DEFAULT BG_DEFAULT) "\n");
    }
    break;


    // decrypt mode
    case WM_DECRYPT:
    {
	// name input
	note = get_file(&file_name, FM_READ);

	// print title
	if (interactive)
	{
	    clear(0, 0);
	    print_title(file_name);
	}
    
	// key input
	key = get_key(key, &key_len, 0);

	

	printf(COLOR(FG_BLACK BG_CYAN_L));
	size_t read_size = read_note(note, key, key_len);
	printf(COLOR(CO_DEFAULT));

	if (read_size == 0)
	    printf("\t" COLOR(FG_BLACK BG_RED) " *** DECRYPT ERROR *** " COLOR(FG_DEFAULT BG_DEFAULT) "\n");
    }
    break;

    default:
	break;
    }






    // free
    
    if (note != NULL)
    {
	fclose(note);
	note = NULL;
    }
    
    free(file_name);
    free(key);
    free(data);


    return 0;
}





size_t get_input(const char * prompt, byte * buf_out, size_t buf_len, int is_retype)
{
    char * password_mismatch = "";
    size_t buf_len_cur = 0,
	buf_len_tmp[2] = {0};
    char * buf_tmp[2];
    buf_tmp[0] = (char *) malloc(sizeof(byte) * (buf_len+1));
    buf_tmp[1] = (char *) malloc(sizeof(byte) * (buf_len+1));

    
    while( 1 )
    {
	buf_len_tmp[0] = 0;
	buf_len_tmp[1] = 0;
	
	// first input
	printf("%s%s: ", password_mismatch, prompt);
	buf_len_tmp[0] = get_stdin(buf_tmp[0], buf_len);
	clear(0, 1);

	// no retype and recheck if set
	if (!is_retype)
	{
	    buf_len_cur = buf_len_tmp[0];
	    break;
	}
	
	// second input
	printf("%sRetype %s: ", password_mismatch, prompt);
	buf_len_tmp[1] = get_stdin(buf_tmp[1], buf_len);
	clear(0, 1);

	if (buf_len_tmp[0] == buf_len_tmp[1] &&
	    memcmp(buf_tmp[0], buf_tmp[1], buf_len_tmp[1]) == 0)
	    break;

	password_mismatch = COLOR(FG_BLACK BG_RED) "[MISMATCH]" COLOR(FG_DEFAULT BG_DEFAULT) " ";
    }


    // copy input to buf
    buf_len_cur = buf_len_tmp[0];
    memcpy(buf_out, buf_tmp[0], buf_len_cur + 1);

    
    free(buf_tmp[0]);
    free(buf_tmp[1]);

    return buf_len_cur;
}

size_t get_stdin(char * buf, size_t buf_len)
{
    fgets((char *)buf, buf_len, stdin);
    
    size_t input_len = strnlen(buf, buf_len);
	
    char * last_element = &buf[input_len - 1];
    if (*last_element == '\n')
    {
	*last_element = '\0';
	input_len--;
    }

    return input_len;
}



FILE * get_file(char ** file_name, fopenmode_t fopenmode)
{
    char * name_buf
	= (char *) malloc(sizeof(char) * BUF_LEN);
    size_t name_len;
    
    if (*file_name == NULL)
    {
	printf("Note name: ");
	fgets(name_buf, BUF_LEN, stdin);
    }
    else
    {
	strncpy(name_buf, *file_name, BUF_LEN);
    }
	
    name_len = strnlen(name_buf, BUF_LEN);
	
    char * last_element = &name_buf[name_len - 1];
    if (*last_element == '\n')
    {
	*last_element = '\0';
	name_len--;
    }

    // append cnote extension to filename
    if (strstr(name_buf, EXT) != (name_buf + name_len - EXT_LEN))
	strcat(name_buf, EXT);
	    
    *file_name = name_buf;

    
    return fopen(*file_name, FOPENMODE_STR[fopenmode]);
}

byte * get_key(byte * key, size_t * key_len_out, int is_retype)
{
    size_t key_len = 0;

    byte * key_buf
	= (byte *) malloc(sizeof(char) * KEY_BUF_LEN);
    
    if (key == NULL)
    {
	key_len = get_input("Key", key_buf, KEY_BUF_LEN, is_retype);
    }
    else
    {
	key_len = strnlen((char *)key, KEY_BUF_LEN);
	strncpy((char *)key_buf, (char *)key, key_len);
    }
    

    if (key_len == 0)
	return NULL;

    key_buf = realloc(key_buf, sizeof(byte) * key_len);
    *key_len_out = key_len;
    return key_buf;
}

byte * get_data(byte * data, size_t * data_len_out)
{
    size_t data_len = 0;

    byte * data_buf
	= (byte *) malloc(sizeof(char) * DATA_BUF_LEN);
    
    if (data == NULL)
    {
	data = data_buf;
		
	data_len = get_input("Data", data, DATA_BUF_LEN, 0);
    }
    else
    {
	data_len = strnlen((const char *)data, DATA_BUF_LEN);
	strncpy((char *)data_buf, (char *)data, data_len);
    }
    

    if (data_len == 0)
	return NULL;

    data_buf = realloc(data_buf, sizeof(byte) * data_len);
    *data_len_out = data_len;
    return data_buf;
}


void print_title(char * title)
{
    printf(MOVE_CURSOR COLOR(FG_BLACK BG_YELLOW_L) " [%s]" FILL_LINE COLOR(CO_DEFAULT) "\n", 1, 1, title);
}
