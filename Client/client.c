#include "includes.h"

struct sockaddr_in client_addr;

int init_connection(char *ip, int port)
{
    int client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if(client_socket < 0)
    {
        perror("[-] socket error");
        exit(0);
    }
    else
        printf("[+] client socket created.\n");

    bzero(&client_addr, sizeof(client_addr));
    client_addr.sin_family = AF_INET;
    client_addr.sin_port = htons(port);
    client_addr.sin_addr.s_addr = inet_addr(ip);

    int cnnct_ret = connect(client_socket, (struct sockaddr*)&client_addr, sizeof(client_addr));
    if(cnnct_ret < 0)
    {
        perror("[-] connect error");
        exit(0);
    }
    else
        printf("[+] connected to the server.\n");

    return client_socket;
}

int main()
{
    int client_socket;
    // initialise connection with the name server
    init_connection(LOCALHOST, DEFAULT_NS_PORT);

    
}