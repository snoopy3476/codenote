#!/bin/bash

PASSPHRASE_HEADER=passphrase.h
PASSPHRASE_LEN=1000


mv "$PASSPHRASE_HEADER" "$PASSPHRASE_HEADER"_bak 2> /dev/null
[ $? == 0 ] && echo "
 - The original '"$PASSPHRASE_HEADER"' is moved to '"$PASSPHRASE_HEADER"_bak'."




echo -n '#ifndef __CODENOTE_PASSPHRASE_H__
#define __CODENOTE_PASSPHRASE_H__

// The postfix of a passphrase
#define PASSPHRASE "' > $PASSPHRASE_HEADER

cat /dev/urandom | tr -dc "A-Za-z0-9~\!\@\#$%^&*()_+\`\-={}|:\"<>[]\;',./" | head -c $PASSPHRASE_LEN | sed 's/"/\\"/g' >> $PASSPHRASE_HEADER

echo '"

#define PASSPHRASE_LEN '$PASSPHRASE_LEN'

#endif' >> $PASSPHRASE_HEADER





echo "
 - A new 'passphrase.h' is created!
 - For compatibility, Make sure to keep same passphrase.h when you compile codenote.
 - All .cnote files from different binaries with different passphrase postfixes WILL NOT BE COMPATIBLE WITH EACH OTHER!

   (Ex. An encrypted .cnote from a binary with postfixes #1, cannot be decrypted by a binary with postfixes #2!)
"
