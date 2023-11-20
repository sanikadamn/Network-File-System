#ifndef NS
#define NS

#include "filemap.h"

/**
 * Defines functions to interact with the naming server. This includes the locks.
 */

struct network {
    char ss_ip[50];
    char ns_ip[50];
    char client_port[5];
    char ns_port[5];
};


extern struct files* ss_files;
extern struct network net_details;

void send_heartbeat (void* arg);

/*
** Prepare file maps into a packet that can be sent over to the naming server
*/
void prepare_filemap_packet (struct files file_map, buf_t* buffer);

#endif // NS_H_
