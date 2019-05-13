#!/bin/bash

SALT_HEADER=salt.h
SALT_LEN=10000


mv "$SALT_HEADER" "$SALT_HEADER"_bak 2> /dev/null
[ $? == 0 ] && echo "
 - The original '"$SALT_HEADER"' is moved to '"$SALT_HEADER"_bak'."




echo -n '#ifndef __CODENOTE_SALT_H__
#define __CODENOTE_SALT_H__

#define KEY_SALT "' > $SALT_HEADER

cat /dev/urandom | tr -dc "A-Za-z0-9~\!\@\#$%^&*()_+\`\-={}|:\"<>[]\;',./" | head -c $SALT_LEN | sed 's/"/\\"/g' >> $SALT_HEADER

echo '"

#define KEY_SALT_LEN '$SALT_LEN'

#endif' >> $SALT_HEADER





echo "
 - A new 'salt.h' is created!
 - For compatibility, Make sure to keep same salt.h when you compile codenote.
 - All .cnote files from different binaries with different salts WILL NOT BE COMPATIBLE WITH EACH OTHER!

   (Ex. An encrypted .cnote from a binary with salt 1, cannot be decrypted by a binary with salt 2!)
"
