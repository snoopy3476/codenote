#include "noteio.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define KEY_BUF_LEN 1 << 15
#define BUF_LEN 1024
#define EXT ".cnote"
#define EXT_LEN 6
#define HKS_BASE "                                                                                                    "
#define HIDE_KEY_STR HKS_BASE HKS_BASE HKS_BASE HKS_BASE HKS_BASE HKS_BASE HKS_BASE HKS_BASE HKS_BASE HKS_BASE

#define new_page() printf("\033[1;1H\033[2J")
#define clear(line_from) printf("\033[%dH\033[J", line_from)
#define cursor_move(x, y) printf("\033[%d;%dH", (x), (y))


typedef enum workmode_t {NONE, ENC, DEC} workmode_t;

size_t get_input(const char * title, const char * prompt, unsigned char * buf_out, size_t buf_len, int is_retype);
size_t get_stdin(unsigned char * buf, size_t buf_len);
int datncmp(unsigned char * buf1, unsigned char * buf2, size_t buf_len);



int main(int argc, char* argv[])
{
    new_page();

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
	char * file_name = NULL;
	char * name_buf
	    = (char *) malloc(sizeof(char) * BUF_LEN);
	size_t name_len;
	
	unsigned char * key = NULL;
	unsigned char * key_buf
	    = (unsigned char *) malloc(sizeof(char) * BUF_LEN);
	size_t key_len;
	
	unsigned char * data = NULL;
	unsigned char * data_buf
	    = (unsigned char *) malloc(sizeof(char) * BUF_LEN);
	size_t data_len;



	
	workmode_t workmode = NONE;
	if (strncmp(argv[1], "-e", BUF_LEN) == 0)
	{
	    workmode = ENC;
	    
	    if (argc >= 3)
		file_name = argv[2];
	    if (argc >= 4)
		key = (unsigned char *) argv[3];
	    if (argc >= 5)
		data = (unsigned char *) argv[4];
	}
	else
	{
	    workmode = DEC;
	    
	    if (argc >= 2)
		file_name = argv[1];
	    if (argc >= 3)
		key = (unsigned char *) argv[2];
	    if (argc >= 4)
		data = (unsigned char *) argv[3];
	}

	


	

    
	

	




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
	    key_len = get_input(file_name, "Key", key_buf, KEY_BUF_LEN, (workmode == ENC));
	    key = key_buf;
	}
	else
	{
	    key_len = strnlen((const char *)key, KEY_BUF_LEN);
	}




	

	// encrypt mode
	if (workmode == ENC)
	{
	    if (data == NULL)
	    {
		data = data_buf;
		
		data_len = get_input(file_name, "Data", data, KEY_BUF_LEN, 0);
	    }
	    else
	    {
		data_len = strnlen((const char *)data, KEY_BUF_LEN);
	    }
	    
	    write_note(file_name, key, key_len, data, data_len);

	    clear(1);

	}


	// decrypt mode
	else if (workmode == DEC)
	{
	    
	    read_note(file_name, key, key_len);

	    
	    clear(3);
	}




	
    }


    
    //load_note(file_name);


    return 0;
}


size_t get_input(const char * title, const char * prompt, unsigned char * buf_out, size_t buf_len, int is_retype)
{
    clear(1);
    printf("[%s]\n", title);

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
	clear(2);
	//printf("\r%.*s\x1B[A\n", (int) (buf_len_tmp[0] + 2 + strnlen(prompt, BUF_LEN) + strnlen(password_mismatch, BUF_LEN)), HIDE_KEY_STR);

	// no retype and recheck if set
	if (!is_retype)
	{
	    buf_len_cur = buf_len_tmp[0];
	    break;
	}
	
	// second input
	printf("Retype %s: ", prompt);
	buf_len_tmp[1] = get_stdin(buf_tmp[1], buf_len);
	clear(2);
	//printf("\r%.*s\x1B[A\n", (int) (buf_len_tmp[1] + 9 + strnlen(prompt, BUF_LEN)), HIDE_KEY_STR);

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
