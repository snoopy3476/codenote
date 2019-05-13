#include "noteio.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define KEY_BUF_LEN 1 << 15
#define BUF_LEN 1024
#define EXT ".cnote"
#define EXT_LEN 6
#define HIDE_KEY_STR "                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                    "


typedef enum workmode_t {NONE, ENC, DEC} workmode_t;

size_t get_password_stdin(char * prompt, unsigned char * buf_out, size_t buf_len, int is_retype);
size_t get_stdin(unsigned char * buf, size_t buf_len);
int datncmp(unsigned char * buf1, unsigned char * buf2, size_t buf_len);



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
	workmode_t workmode = NONE;
	if (strncmp(argv[1], "enc", BUF_LEN) == 0)
	    workmode = ENC;
	else if (strncmp(argv[1], "dec", BUF_LEN) == 0)
	    workmode = DEC;

	if ( workmode == NONE )
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

	
	char name_buf[BUF_LEN + EXT_LEN] = {0};
	size_t name_len;
	unsigned char key_buf[KEY_BUF_LEN] = {0};
	size_t key_len;




	// name input
	if (file_name == NULL)
	{
	    printf("Note name: ");
	    fgets(name_buf, BUF_LEN, stdin);
	}
	else
	{
	    strncpy(name_buf, file_name, BUF_LEN);
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
	    
	file_name = name_buf;




	// key input
	if (key == NULL)
	{
	    key_len = get_password_stdin("Key", key_buf, KEY_BUF_LEN, (workmode == ENC));
	    key = key_buf;
	}
	else
	{
	    key_len = strnlen((const char *)key, KEY_BUF_LEN);
	}




	

	// encrypt mode
	if (workmode == ENC)
	{
	    unsigned char data_buf[KEY_BUF_LEN] = {0};
	    size_t data_len;

	    if (data == NULL)
	    {
		data = data_buf;
		
		data_len = get_password_stdin("Data", data, KEY_BUF_LEN, 0);
	    }
	    else
	    {
		data_len = strnlen((const char *)data, KEY_BUF_LEN);
	    }
	    
	    write_note(file_name, key, key_len, data, data_len);


	    return 0;
	}


	// decrypt mode
	else if (workmode == DEC)
	{
	    read_note(file_name, key, key_len);
	}
    }


    
    //load_note(file_name);


    return 0;
}


size_t get_password_stdin(char * prompt, unsigned char * buf_out, size_t buf_len, int is_retype)
{

    char * password_mismatch = "";
    size_t buf_len_cur = 0,
	buf_len_tmp[2] = {0};
    unsigned char * buf_tmp[2];
    buf_tmp[0] = (unsigned char *) malloc(sizeof(unsigned char) * (buf_len+1));
    buf_tmp[1] = (unsigned char *) malloc(sizeof(unsigned char) * (buf_len+1));

    
    while( 1 )
    {
	buf_len_tmp[0] = 0;
	buf_len_tmp[1] = 0;

	// first input
	printf("%s%s: ", password_mismatch, prompt);
	buf_len_tmp[0] = get_stdin(buf_tmp[0], buf_len);
	printf("\r%.*s\x1B[A\n", (int) (buf_len_tmp[0] + 2 + strnlen(prompt, BUF_LEN) + strnlen(password_mismatch, BUF_LEN)), HIDE_KEY_STR);

	// no retype and recheck if set
	if (!is_retype)
	{
	    buf_len_cur = buf_len_tmp[0];
	    break;
	}
	
	// second input
	printf("Retype %s: ", prompt);
	buf_len_tmp[1] = get_stdin(buf_tmp[1], buf_len);
	printf("\r%.*s\x1B[A\n", (int) (buf_len_tmp[1] + 9 + strnlen(prompt, BUF_LEN)), HIDE_KEY_STR);

	if (buf_len_tmp[0] == buf_len_tmp[1] &&
	    datncmp(buf_tmp[0], buf_tmp[1], buf_len_tmp[1]) == 0)
	    break;

	password_mismatch = "[MISMATCH] ";

	if (!feof(stdin))
	    return 0;
    }


    // copy input to buf
    buf_len_cur = buf_len_tmp[0];
    memcpy(buf_out, buf_tmp[0], buf_len_cur + 1);

    
    free(buf_tmp[0]);
    free(buf_tmp[1]);

    return buf_len_cur;
}

size_t get_stdin(unsigned char * buf, size_t buf_len)
{
    
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

    return buf_len_cur;
}

int datncmp(unsigned char * buf1, unsigned char * buf2, size_t buf_len)
{
    for (size_t i = 0; i < buf_len; i++)
	if (buf1[i] - buf2[i])
	    return buf1[i] - buf2[i];

    return 0;
}
