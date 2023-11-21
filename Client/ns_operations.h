#ifndef __CLIENT_NAME_SERVER_OPERATIONS_H
#define __CLIENT_NAME_SERVER_OPERATIONS_H

#include "includes.h"

packet_d ns_expect_redirect(char *action, char *file);
void ns_expect_feedback(char *action, char *file1, char *file2);

#endif