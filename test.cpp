#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdbool.h>
#include <limits.h>

#define SERVERPORT 8989
#define BUFSIZE 4096
#define SOCKETERROR (-1)
#define SERVER_BACKLOG 1


// typedef allows us to use these, with a more meaningful name
typedef struct sockaddr_in SA_IN;
typedef struct sockaddr SA;

// here for linking (or compiling), actual implimintation is down.
void handle_connection(int client_socket);
// chack is a general "catch error" function. most of the build in socket function return the same values for 
//seccses and failing (-1). 
int check(int exp, const char *msg);

int main(int argc , char **argv)
{
    int server_socket, client_socket, addr_size;
    SA_IN server_addr, client_addr;

    // craete the socket: AF_INET = IPv4, SOCK_STREAM = TCP, 0 = unspecified default protocol appropriate for the requested socket type.
    check((server_socket = socket(AF_INET, SOCK_STREAM, 0)), 
            "Failed to create socket");

    //initializing the address struct
    server_addr.sin_family = AF_INET; // IPv4
    server_addr.sin_addr.s_addr = INADDR_ANY; // use my IPv4 address
    server_addr.sin_port = htons(SERVERPORT); // Host TO Network Short
    
    //understand bind!
    check((bind(server_socket, (SA*)&server_addr, sizeof(server_addr))), 
        "Bind Failed");
}