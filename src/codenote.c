#include "noteio.h"
#include "theme/ansiseq.h"
#include "theme/theme.h"
#include "bytedata/bytedata.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BUF_LEN 1024
#define BUF_SIZE_LONG (1 << 20)

#define EXT ".cnote"
#define EXT_LEN 6



#define MSG_ERR_UNDEFINED "UNDEFINED ERROR"
#define MSG_ERR_FOPEN "FILE OPEN ERROR"
#define MSG_ERR_STDIN "INPUT ERROR"
#define MSG_ERR_ENCRYPT "ENCRYPT ERROR"
#define MSG_ERR_DECRYPT "DECRYPT ERROR"



#define new_page() printf(MOVE_CURSOR NEWPAGE, 1, 1); fflush(stdout)
#define clear(offset_x, offset_y) printf(MOVE_CURSOR CLEAR, (offset_y)+1, (offset_x)+1); fflush(stdout)



typedef enum workmode_t {WM_NONE, WM_ENCRYPT, WM_DECRYPT} workmode_t;
typedef enum fopenmode_t {FM_READ = 0, FM_WRITE} fopenmode_t;

typedef unsigned char byte;

typedef struct data_t
{
    byte * data;
    size_t size;
} data_t;




FILE * get_file(char ** file_name, fopenmode_t fopenmode);

byte * get_data(const char * prompt, byte * data, size_t * data_size_out, int is_retype);
size_t get_input(const char * prompt, byte ** buf_out, int is_retype);

void print_title(workmode_t workmode, char * title);

bytedata load_file_data(FILE * fp, char const * const delimiters);
size_t load_file_data_wrapper(FILE * fp, byte ** out, char const * const delimiters);



int main(int argc, char* argv[])
{
    char * file_name = NULL;
    byte * passphrase = NULL;
    size_t passphrase_size;
    byte * data = NULL;
    size_t data_size;
    int interactive = 1;



    

    // init //
    
    // use ANSI sequence on Windows 10
    set_windows_ansi_ready();


    


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
	    passphrase = (byte *) argv[3];
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
	    
	    passphrase = (byte *) argv[2];
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
    
	// passphrase input
	passphrase = get_data("Passphrase", passphrase, &passphrase_size, 1);
	if (passphrase == NULL)
	{
	    error_msg = MSG_ERR_STDIN;
	    fclose(note);
	    break;
	}

	// data input
	data = get_data("Data", data, &data_size, 0);
	if (data == NULL)
	{
	    error_msg = MSG_ERR_STDIN;
	    fclose(note);
	    break;
	}

	
	// encrypt data and save to file
	error_msg = MSG_ERR_ENCRYPT;
	processed_size = write_note(note, passphrase, passphrase_size, data, data_size);
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
	processed_size = read_note(note_check, passphrase, passphrase_size, &processed_data);
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
    
	// passphrase input
	passphrase = get_data("Passphrase", passphrase, &passphrase_size, 0);
	if (passphrase == NULL)
	{
	    error_msg = MSG_ERR_STDIN;
	    fclose(note);
	    break;
	}

	
	// decrypt data
	error_msg = MSG_ERR_DECRYPT;
	processed_size = read_note(note, passphrase, passphrase_size, &processed_data);
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
	printf(COLOR(COLOR_WARNING) " *** %s *** " FILL_LINE COLOR(CO_DEFAULT) "\n", error_msg);
	footer_line = ""; // print red line instead of data line
    }

    // footer bar
    if (interactive)
	printf("%s" COLOR(COLOR_TITLE) FILL_LINE COLOR(CO_DEFAULT) "\n", footer_line);






    // clean up //
    
    free(file_name);
    free(passphrase);
    free(data);





    // prevent automatic program exit if interactive
    if (interactive)
    {
	printf("Press Enter to Exit...");
	byte buf[BUF_LEN];
	fgets((char *)buf, BUF_LEN, stdin);
    }

    if (processed_size != 0)
	return 0;
    else
	return -1;
}






// open file from filename
char const * const FOPENMODE_STR[2] =
{
    "rb",
    "wb+"
};

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



// get data (stdin)
byte * get_data(const char * prompt, byte * data, size_t * data_size_out, int is_retype)
{
    size_t data_size = 0;

    byte * data_buf = NULL;
    
    if (data == NULL)
    {
	data_size = get_input(prompt, &data_buf, is_retype);
    }
    else
    {
	data_buf = (byte *) malloc(sizeof(char) * BUF_SIZE_LONG);
	data_size = strnlen((const char *)data, BUF_SIZE_LONG);
	strncpy((char *)data_buf, (char *)data, data_size);
    }
    

    if (data_size == 0)
    {
	if (data_buf != NULL)
	    free(data_buf);
	
	return NULL;
    }

    *data_size_out = data_size;
    
    return data_buf;
}


// get input of passphrase / data (stdin)
size_t get_input(const char * prompt, byte ** buf_out, int is_retype)
{
    char * password_mismatch = "";
    size_t buf_len_cur = 0,
	buf_len_tmp[2] = {0};
    byte * buf_tmp[2] = {NULL, NULL};

    
    while( 1 )
    {
	buf_len_tmp[0] = 0;
	buf_len_tmp[1] = 0;
	
	// first input
	printf("%s" COLOR(COLOR_DATA_BG) "%s:" COLOR(CO_DEFAULT) " ", password_mismatch, prompt);
	fflush(stdout); // always print prompt above
	buf_len_tmp[0] = load_file_data_wrapper(stdin, &buf_tmp[0], "\0\n");
	clear(0, 1);

	// no retype and recheck if set
	if (!is_retype)
	{
	    buf_len_cur = buf_len_tmp[0];
	    break;
	}
	
	// second input
	printf("%s"  COLOR(COLOR_DATA_BG) "Retype %s:" COLOR(CO_DEFAULT) " ", password_mismatch, prompt);
	fflush(stdout); // always print prompt above
	buf_len_tmp[1] = load_file_data_wrapper(stdin, &buf_tmp[1], "\0\n");
	clear(0, 1);

	if (buf_len_tmp[0] == buf_len_tmp[1] &&
	    memcmp(buf_tmp[0], buf_tmp[1], buf_len_tmp[1]) == 0)
	    break;

	password_mismatch = COLOR(COLOR_WARNING) "[MISMATCH]" COLOR(COLOR_DATA_BG) " ";
    }


    // copy input to buf
    buf_len_cur = buf_len_tmp[0];
    *buf_out = buf_tmp[0];
    
    if (buf_tmp[1] != NULL)
	free(buf_tmp[1]);

    return buf_len_cur;
}




// print title bar
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




// load file data wrapper
size_t load_file_data_wrapper(FILE * fp, byte ** out, char const * const delimiters)
{
    bytedata file_data;
    file_data = load_file_data(fp, delimiters);


    *out = (byte *) malloc(sizeof(byte) * get_bytedata_size(file_data));
    memcpy(*out, get_bytedata(file_data), get_bytedata_size(file_data));

    return get_bytedata_size(file_data);
}
