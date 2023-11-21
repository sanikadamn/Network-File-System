#define OK 0
#define ENOSERV 1
#define ENOPERM 2
#define EINVAL 3
#define EFULL 4
#define ENOTFOUND 5
#define EBUSY 6
#define EFSERROR 7

#define SEND_STATUS(sock, x) send(sock, "STATUS:" #x "\n", 7 + sizeof(#x), 0)
