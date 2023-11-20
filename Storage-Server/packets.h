#ifndef SS_PACKETS
#define SS_PACKETS

#include "filemap.h"


enum packet_type {
OPEN = 0,
CLOSE,
READ,
WRITE,
DELETE,
COPYIN,
COPYOUT
};

void respond_read (int fd);

extern struct files* ss_files;

#endif // PACKETS
