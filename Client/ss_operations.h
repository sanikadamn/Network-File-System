#ifndef __SS_OPERATIONS_H
#define __SS_OPERATIONS_H

#include "includes.h"

void ss_connect(char *ip, int port);
void ss_read_req(char *action, char *filepath);
void ss_write_req(char *action, char *filepath);
void ss_info_req(char *action, char *filepath);

#endif