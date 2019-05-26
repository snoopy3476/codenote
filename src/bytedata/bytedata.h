#ifndef __CODENOTE_BYTEDATA_H__
#define __CODENOTE_BYTEDATA_H__

#include <stddef.h>


typedef unsigned char byte;

typedef struct bytedata
{
    size_t size;
    byte data[]; // variable length
} * bytedata; // struct bytedata_orig * => bytedata



bytedata new_bytedata(size_t size);
bytedata resize_bytedata(bytedata * data, size_t size);
void free_bytedata(bytedata data);

#define get_bytedata_size(data_input) ((data_input)->size)
#define get_bytedata(data_input) ((data_input)->data)
bytedata set_bytedata(bytedata * data, const byte * new_data, size_t size);

bytedata append_bytedata(bytedata * data, const byte * new_data, size_t size);
bytedata pop_bytedata(bytedata * data, byte * data_out, size_t size);



#endif
