#include "noteio.h"
#include "ansiseq.h"
#include "theme.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BUF_LEN 1024
#define KEY_BUF_SIZE (1 << 15)
#define DATA_BUF_SIZE (1 << 20)

#define EXT ".cnote"
#define EXT_LEN 6

#define HKS_BASE "                                                                                                    "
#define HIDE_KEY_STR HKS_BASE HKS_BASE HKS_BASE HKS_BASE HKS_BASE HKS_BASE HKS_BASE HKS_BASE HKS_BASE HKS_BASE


#define MSG_ERR_UNDEFINED "UNDEFINED ERROR"
#define MSG_ERR_FOPEN "FILE OPEN ERROR"
#define MSG_ERR_STDIN "INPUT ERROR"
#define MSG_ERR_ENCRYPT "ENCRYPT ERROR"
#define MSG_ERR_DECRYPT "DECRYPT ERROR"



#define new_page() printf(MOVE_CURSOR NEWPAGE, 1, 1)
#define clear(offset_x, offset_y) printf(MOVE_CURSOR CLEAR, (offset_y)+1, (offset_x)+1)



typedef enum workmode_t {WM_NONE, WM_ENCRYPT, WM_DECRYPT} workmode_t;
typedef enum fopenmode_t {FM_READ = 0, FM_WRITE} fopenmode_t;
char const * const FOPENMODE_STR[2] =
{
    "rb",
    "wb+"
};




FILE * get_file(char ** file_name, fopenmode_t fopenmode);

byte * get_key(byte * key, size_t * key_size_out, int is_retype);
byte * get_data(byte * data, size_t * data_size_out);
size_t get_input(const char * prompt, byte * buf_out, size_t buf_len, int is_retype);
size_t get_stdin(char * buf, size_t buf_len);

void print_title(workmode_t workmode, char * title);



int main(int argc, char* argv[])
{
    char * file_name = NULL;
    byte * key = NULL;
    size_t key_size;
    byte * data = NULL;
    size_t data_size;
    int interactive = 1;



    

    // init //
    
    // use ANSI sequence on Windows 10
    set_windows_ansi_ready();

    // set seed for random iv
    rand_seed();


    


    // args processing //
    
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




    
    // print title //
    
    if (interactive)
    {
	new_page();
	clear(0, 0);
	print_title(workmode, NULL);
    }
	

	



    // encrypt || decrypt //
    
    size_t processed_size = 0;
    byte * processed_data = NULL;
    char * error_msg = MSG_ERR_UNDEFINED;
    int print_decrypted_data = 1;

    switch (workmode)
    {

    // encrypt mode
    case WM_ENCRYPT:
    {
	// name input
	FILE * note = get_file(&file_name, FM_WRITE);
	if (note == NULL)
	{
	    error_msg = MSG_ERR_FOPEN;
	    break;
	}

	// print title
	if (interactive)
	{
	    clear(0, 0);
	    print_title(workmode, file_name);
	}
    
	// key input
	key = get_key(key, &key_size, 1);
	if (key == NULL)
	{
	    error_msg = MSG_ERR_STDIN;
	    fclose(note);
	    break;
	}

	// data input
	data = get_data(data, &data_size);
	if (data == NULL)
	{
	    error_msg = MSG_ERR_STDIN;
	    fclose(note);
	    break;
	}

	
	// encrypt data and save to file
	error_msg = MSG_ERR_ENCRYPT;
	processed_size = write_note(note, key, key_size, data, data_size);
	fclose(note);
	note = NULL;


	// check if encrypted without error
	FILE * note_check = get_file(&file_name, FM_READ);
	if (note_check == NULL)
	    break;


	// don't print checked data when non-interactive
	if (!interactive)
	    print_decrypted_data = 0;


	// check note if encrypted properly
	processed_size = read_note(note_check, key, key_size, &processed_data);
	if (data_size != processed_size || memcmp(data, processed_data, data_size) != 0)
	{
	    free(processed_data);
	    processed_data = NULL;
	    processed_size = 0;
	}

	fclose(note_check);
    }
    break;


    
    // decrypt mode
    case WM_DECRYPT:
    {
	// name input
	FILE * note = get_file(&file_name, FM_READ);
	if (note == NULL)
	{
	    error_msg = MSG_ERR_FOPEN;
	    break;
	}

	// print title
	if (interactive)
	{
	    clear(0, 0);
	    print_title(workmode, file_name);
	}
    
	// key input
	key = get_key(key, &key_size, 0);
	if (key == NULL)
	{
	    error_msg = MSG_ERR_STDIN;
	    fclose(note);
	    break;
	}

	
	// decrypt data
	error_msg = MSG_ERR_DECRYPT;
	processed_size = read_note(note, key, key_size, &processed_data);
	fclose(note);
	note = NULL;

    }
    break;

    default:
	break;
    }



    


    
    // print result //
    
    char * footer_line = COLOR(COLOR_DATA_BG) FILL_LINE "\n";

    // title bar
    if (interactive)
	printf(COLOR(COLOR_DATA) MOVE_CURSOR, 2, 1);

    // processed properly
    if (processed_data != NULL)
    {
	if ( print_decrypted_data )
	    for (size_t index = 0; index < processed_size; index++)
		printf("%c", processed_data[index]);
	
	free(processed_data);
    }

    // error on processing
    else
    {
	printf(COLOR(COLOR_WARNING) " *** %s *** " FILL_LINE "\n" COLOR(CO_DEFAULT), error_msg);
	footer_line = ""; // print red line instead of data line
    }

    // footer bar
    if (interactive)
	printf("%s" COLOR(COLOR_TITLE) FILL_LINE "\n" COLOR(CO_DEFAULT), footer_line);






    // clean up //
    
    free(file_name);
    free(key);
    free(data);


    if (processed_size != 0)
	return 0;
    else
	return -1;
}






// open file from filename
FILE * get_file(char ** file_name, fopenmode_t fopenmode)
{
    char * name_buf
	= (char *) malloc(sizeof(char) * BUF_LEN);
    size_t name_len;
    
    if (file_name == NULL || *file_name == NULL)
    {
	printf(COLOR(COLOR_DATA_BG) "Note name:" COLOR(CO_DEFAULT) " ");
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

    
    if (file_name == NULL)
	free(name_buf);
    else
	*file_name = name_buf;

    
    return fopen(*file_name, FOPENMODE_STR[fopenmode]);
}





// get key (stdin)
byte * get_key(byte * key, size_t * key_size_out, int is_retype)
{
    size_t key_size = 0;

    byte * key_buf
	= (byte *) malloc(sizeof(char) * KEY_BUF_SIZE);
    
    if (key == NULL)
    {
	key_size = get_input("Key", key_buf, KEY_BUF_SIZE, is_retype);
    }
    else
    {
	key_size = strnlen((char *)key, KEY_BUF_SIZE);
	strncpy((char *)key_buf, (char *)key, key_size);
    }
    

    if (key_size == 0)
	return NULL;

    byte * key_buf_realloc = realloc(key_buf, sizeof(byte) * key_size);
    if (key_buf_realloc == NULL)
    {
	free(key_buf);
	return NULL;
    }

    key_buf = key_buf_realloc;
    *key_size_out = key_size;
    
    return key_buf;
}

// get data (stdin)
byte * get_data(byte * data, size_t * data_size_out)
{
    size_t data_size = 0;

    byte * data_buf
	= (byte *) malloc(sizeof(char) * DATA_BUF_SIZE);
    
    if (data == NULL)
    {
	data = data_buf;
		
	data_size = get_input("Data", data, DATA_BUF_SIZE, 0);
    }
    else
    {
	data_size = strnlen((const char *)data, DATA_BUF_SIZE);
	strncpy((char *)data_buf, (char *)data, data_size);
    }
    

    if (data_size == 0)
	return NULL;

    byte * data_buf_realloc = realloc(data_buf, sizeof(byte) * data_size);
    if (data_buf_realloc == NULL)
    {
	free(data_buf);
	return NULL;
    }

    data_buf = data_buf_realloc;
    *data_size_out = data_size;
    
    return data_buf;
}


// get input of key / data (stdin)
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
	printf("%s" COLOR(COLOR_DATA_BG) "%s:" COLOR(CO_DEFAULT) " ", password_mismatch, prompt);
	buf_len_tmp[0] = get_stdin(buf_tmp[0], buf_len);
	clear(0, 1);

	// no retype and recheck if set
	if (!is_retype)
	{
	    buf_len_cur = buf_len_tmp[0];
	    break;
	}
	
	// second input
	printf("%s"  COLOR(COLOR_DATA_BG) "Retype %s:" COLOR(CO_DEFAULT) " ", password_mismatch, prompt);
	buf_len_tmp[1] = get_stdin(buf_tmp[1], buf_len);
	clear(0, 1);

	if (buf_len_tmp[0] == buf_len_tmp[1] &&
	    memcmp(buf_tmp[0], buf_tmp[1], buf_len_tmp[1]) == 0)
	    break;

	password_mismatch = COLOR(COLOR_WARNING) "[MISMATCH]" COLOR(COLOR_DATA_BG) " ";
    }


    // copy input to buf
    buf_len_cur = buf_len_tmp[0];
    memcpy(buf_out, buf_tmp[0], buf_len_cur + 1);

    
    free(buf_tmp[0]);
    free(buf_tmp[1]);

    return buf_len_cur;
}


// get input from stdin
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





char const * const WORKMODE_STR[3] =
{
    "GENERAL Mode",
    COLOR(COLOR_TITLE_RED) "ENCRYPT Mode",
    "DECRYPT Mode"
};

void print_title(workmode_t workmode, char * title)
{
    printf(MOVE_CURSOR COLOR(COLOR_TITLE) " [Codenote] %s " COLOR(COLOR_TITLE), 1, 1, WORKMODE_STR[workmode]);
    if (title != NULL)
	printf("- " COLOR(COLOR_TITLE_FILE_NAME) "<%s>" COLOR(COLOR_TITLE), title);
    printf(FILL_LINE COLOR(CO_DEFAULT) "\n");
}
