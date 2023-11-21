#include "includes.h"

int client_ns_socket = -1;
int client_ss_socket = -1;
struct sockaddr_in client_addr = {0};

void init_connection(char *ip, int port)
{
    client_ns_socket = socket(AF_INET, SOCK_STREAM, 0);
    if(client_ns_socket < 0)
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

    int cnnct_ret = connect(client_ns_socket, (struct sockaddr*)&client_addr, sizeof(client_addr));
    if(cnnct_ret < 0)
    {
        perror("[-] connect error");
        exit(0);
    }
    else
        printf("[+] connected to the server.\n");
}

char* read_line(int fd, int max_len) {
    char* str = malloc(sizeof(char) * (max_len + 1));
    if (str == NULL) return str;
    char* head = str;
    int err;
    int err;
	char ch;
	while ((err = recv(fd, &ch, sizeof(char), 0)) > 0) {
        if(err < 0)
	while ((err = recv(fd, &ch, sizeof(char), 0)) > 0) {
        if(err < 0)
        {
            perror("[-] recv error");
            exit(0);
        }
		if (ch == '\n' || (head - str) == max_len) {
            *head++ = '\0';
			break;
		}
        *head++ = ch;
	}

    int len = head - str;
    str = realloc(str, len);
    return str;
}

int main()
{
    // initialise connection with the name server
    // init_connection(LOCALHOST, DEFAULT_NS_PORT);

    // buf_t *ns_res = creq_to_ns("REQUEST", "test.txt");
    // if(validate_ns_response(ns_res) == -1)
        // exit(0);

    ss_connect(LOCALHOST, DEFAULT_SS_PORT);
    char *choice = (char *)malloc(sizeof(char) * 10);
    char *filepath = (char *)malloc(sizeof(char) * 1024);
    printf("Enter your choice: ");
    scanf("%s", choice);
    printf("Enter the filepath: ");
    scanf("%s", filepath);
    if(strcmp(choice, "read") == 0)
    {
        ss_read_req(choice, filepath);
    }
    // else if(strcmp(choice, "write") == 0)
    // {
    //     ss_write_req(choice, filepath);
    // }
    else if(strcmp(choice, "info") == 0)
    {
        ss_info_req(choice, filepath);
    }
}
