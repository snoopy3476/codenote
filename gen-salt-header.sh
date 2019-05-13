#!/bin/bash

SALT_HEADER=salt.h
SALT_LEN=10000




echo -n '#ifndef __CODENOTE_SALT_H__
#define __CODENOTE_SALT_H__

#define KEY_SALT "' > $SALT_HEADER

cat /dev/urandom | tr -dc "A-Za-z0-9~\!\@\#$%^&*()_+\`\-={}|:\"<>[]\;',./" | head -c $SALT_LEN | sed 's/"/\\"/g' >> $SALT_HEADER

echo '"

#define KEY_SALT_LEN '$SALT_LEN'

#endif' >> $SALT_HEADER
