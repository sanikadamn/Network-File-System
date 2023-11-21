#ifndef __CLIENT_NAME_SERVER_OPERATIONS_H
#define __CLIENT_NAME_SERVER_OPERATIONS_H

#include "includes.h"

packet_d ns_expect_redirect(char *action, char *file);
packet_c ns_expect_feedback(char *action, char *file);

#endif