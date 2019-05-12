#include "noteio.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define KEY_BUF_LEN 1 << 15
#define BUF_LEN 1024
#define HIDE_KEY_STR "                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                    "


typedef enum workmode_t {NONE, ENCODE, DECODE} workmode_t;

size_t get_password_stdin(char * prompt, unsigned char * buf, size_t buf_len);




int main(int argc, char* argv[])
{
    

    // no arg : encrypt mode
    if (argc <= 1)
    {
	
	char argv_buf[1024];
    
	printf("Note name to write: ");
	fgets(argv_buf, BUF_LEN, stdin);
	char * last_element = &argv_buf[strnlen(argv_buf, BUF_LEN) - 1];
	if (*last_element == '\n')
	    *last_element = '\0';
    }

    // arg : decrypt mode
    else
    {
	workmode_t workmode = 0;
	if (strncmp(argv[1], "encode", BUF_LEN) == 0)
	    workmode = 1;
	else if (strncmp(argv[1], "decode", BUF_LEN) == 0)
	    workmode = 2;

	if ( workmode == 0 )
	    return 1;


	

    
	char * file_name = NULL;
	unsigned char * key = NULL;
	unsigned char * data = NULL;
	
	if (argc >= 3)
	    file_name = argv[2];
	if (argc >= 4)
	    key = (unsigned char *) argv[3];
	if (argc >= 5)
	    data = (unsigned char *) argv[4];

	
	char name_buf[BUF_LEN] = {0};
	unsigned char key_buf[KEY_BUF_LEN] = {0};
	size_t key_len;


	if (file_name == NULL)
	{
	    printf("Note name: ");
	    fgets(name_buf, BUF_LEN, stdin);
	    char * last_element = &name_buf[strnlen(name_buf, BUF_LEN) - 1];
	    if (*last_element == '\n')
		*last_element = '\0';

	    file_name = name_buf;
	}


	    
	if (key == NULL)
	{
	    key_len = get_password_stdin("Key: ", key_buf, KEY_BUF_LEN);
	    key = key_buf;
	}
	else
	{
	    key_len = strnlen((const char *)key, KEY_BUF_LEN);
	}




	// encrypt mode
	if (workmode == 1)
	{
	    unsigned char data_buf[KEY_BUF_LEN] = {0};
	    size_t data_len;

	    if (data == NULL)
	    {
		data = data_buf;
		
		data_len = get_password_stdin("Data: ", data, KEY_BUF_LEN);
	    }
	    else
	    {
		data_len = strnlen((const char *)data, KEY_BUF_LEN);
	    }
	    
	    write_note(file_name, key_buf, key_len, data, data_len);


	    return 0;
	}


	// decrypt mode
	else if (workmode == 2)
	{
	    read_note(file_name, key_buf, key_len);
	}
    }


    
    //load_note(file_name);


    return 0;
}


size_t get_password_stdin(char * prompt, unsigned char * buf, size_t buf_len)
{
    printf("%s", prompt);

    
    size_t buf_len_cur = 0;
    char char_buf;
    int get_next_char = 1;
    
    while (get_next_char && (buf_len_cur < buf_len))
    {
	char_buf = fgetc(stdin);
	switch (char_buf)
	{
	case '\n':
	    printf("\x1B[A");
	case '\0':
	case EOF:
	    get_next_char = 0;
	    break;

	default:
	    buf[buf_len_cur++] = char_buf;
	}
    }

    printf("\r%.*s\r", (int) (buf_len_cur + strnlen(prompt, BUF_LEN)), HIDE_KEY_STR);

    return buf_len_cur;
}
