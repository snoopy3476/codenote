#include "bytedata.h"

#include <stdlib.h>
#include <string.h>



#define data_obj_size(size) (sizeof(struct bytedata) + sizeof(byte) * ((size)+1))




bytedata new_bytedata(size_t size)
{
    return resize_bytedata(NULL, size);
}

bytedata resize_bytedata(bytedata * data, size_t size)
{
    bytedata result;
    if (data != NULL)
    {
	result = (bytedata) realloc(*data, data_obj_size(size));
    }
    else
    {
	result = (bytedata) malloc(data_obj_size(size));
	data = &result;
    }
    
    if (result == NULL)
	return NULL;
    *data = result;
    
    result->data[size] = '\0';
    result->size = sizeof(byte) * size;
    return result;
}

void free_bytedata(bytedata data)
{
    if (data != NULL)
	free(data);
}




bytedata set_bytedata(bytedata * data, const byte * new_data, size_t size)
{
    bytedata result = resize_bytedata(data, size);
    if (result == NULL)
	return NULL;

    if (data == NULL)
	data = &result;
    else
	*data = result;

    memcpy(result->data, new_data, sizeof(byte) * size);
    
    return result;
}

bytedata append_bytedata(bytedata * data, const byte * new_data, size_t size)
{
    if (data == NULL)
	return NULL;
    
    size_t data_orig_size;
    if (*data == NULL)
	data_orig_size = 0;
    else
	data_orig_size = get_bytedata_size(*data);

    
    bytedata result = resize_bytedata(data, data_orig_size + size);
    if (result == NULL)
	return NULL;
    *data = result;

    memcpy(result->data + data_orig_size, new_data, sizeof(byte) * size);
    
    return result;
}

bytedata pop_bytedata(bytedata * data, byte * data_out, size_t size)
{
    if (data == NULL || *data == NULL)
	return NULL;
    
    size_t data_orig_size = get_bytedata_size(*data);
    if (data_orig_size < size)
	return NULL;

    if (data_out != NULL)
	memcpy(data_out, get_bytedata(*data) + data_orig_size - size, size);
    
    bytedata result = resize_bytedata(data, data_orig_size - size);
    if (result == NULL)
	return NULL;
    *data = result;
    
    return result;
}
