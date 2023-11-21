#ifndef NEW_PACKETS_H
#define NEW_PACKETS_H
/* going to define all the new packets here */
#include "globals.h"
/* type A
** broad c->ns packets
** they work for: read, write, info, create, delete
*/
typedef struct {
    char action[MAX_ACTION_LENGTH];
    char filename[MAX_FILENAME_LENGTH];
    int numbytes; // required only for write request
    int permissions; // required only for create request
} packet_a;

/* type B
** ss init packets
** has been done
*/

/* type C
** feedback packets
*/
typedef struct {
    int status; // specify different codes depending on the sender, receiver, and action for which feedback is being sent
    int numbytes; // required only for read request's feedback
    int permissions;
} packet_c;

/* type D
** redirect packets (from ns to ss)
*/
typedef struct {
    int status;
    char ip[MAX_IP_LENGTH];
    int port;
} packet_d;

#endif