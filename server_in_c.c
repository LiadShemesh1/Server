#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <stdbool.h>
#include <limits.h>
#include <linux/limits.h>
#include <netdb.h>

#define SERVERPORT 8989  // the port users will be connecting to
#define BUFSIZE 4096  //todo why this size???
#define SOCKETERROR (-1)
#define SERVER_BACKLOG 1 // how many pending connections queue will hold


// typedef allows us to use these, with a more meaningful name
typedef struct sockaddr SA;
typedef struct sockaddr_storage SAS; 
typedef struct addrinfo ADDINFO;
typedef struct sockaddr_in SA_IN;

// here for linking (or compiling), actual implimintation is down.
void handle_connection(int client_socket);
// chack is a general "catch error" function. most of the build in socket function return the same values for 
//seccses and failing (-1). 
int check(int exp, const char *msg);

int main(int argc , char **argv)
{

    int server_socket, client_socket; // client_socket not sure yet
    SA_IN server_addr, client_addr;
    socklen_t addr_size;
    
    check((server_socket = socket(AF_INET, SOCK_STREAM, 0)), 
            "Failed to create socket");
    

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(SERVERPORT);     // short, network byte order


    check(bind(server_socket, (SA*)&server_addr, sizeof(server_addr)), 
            "Bind Failed!");  
    
    check(listen(server_socket, SERVER_BACKLOG),
            "Listen Failed");

    
    while(true)
    {
        printf("waiting for connections... \n");
        addr_size = sizeof(SA_IN); // chaeck
        
        check(client_socket = 
                accept(server_socket, (SA*)&client_addr, &addr_size),
                "Accept Faild");
        printf("Connected \n");
        

        // do whatever we want with the connection:
        handle_connection(client_socket);
    } // while

    return 0;
}

int check(int exp, const char *msg)
{
        if(exp == SOCKETERROR){
                perror(msg);
                exit(1);
        }
        return exp;
}

void handle_connection(int client_socket)
{
        char buffer[BUFSIZE]; // The size of each buffer we will read
        size_t bytes_read;
        int msgsize = 0;
        char actualpath[PATH_MAX + 1];  // +1 is for the null termination character.


        // read the client's message -- the name of the file to read

        /*
        -File to read - or socket represented by int.

        -The place in memory where we read the data (buffer indecates the place and + msgsize is the
        offset - if we read 100 bytes in the first call, in the second read we will write to the place
        in memory where buffer points to plus(+) msgsize which will increament with each read)

        -The length in bytes of the buffer pointed to by the buf parameter. 
        (get smaller with reach read to not exicid the size of buffer and the minus(-) 1 is
        for the null terminator).
        */

        while((bytes_read = read(client_socket, buffer+msgsize, sizeof(buffer)-msgsize-1)) > 0) {
                msgsize += bytes_read;
                if(msgsize > BUFSIZE-1 || buffer[msgsize-1] == '\n') break; //if the last char is '\n' we finished the read
        }
        check(bytes_read, "recive error");
        buffer[msgsize-1] = 0; //null terminate the message and remove the \n


        printf("REQUEST: %s\n", buffer);

        fflush(stdout); // makes sure that all the recived data was written.
        // fflush(stdout) checks if there are any data in the buffer that should be written and if so,
        // the underlying syscall is used to write the data to the OS.
        /*
        fflush() is typically used for output stream only. Its purpose is to clear (or flush)
        the output buffer and move the buffered data to console (in case of stdout) 
        or disk (in case of file output stream)
        */

        //validity check, checks if the file exist as well
        //need to replay with an http error 404 if happened
        if(realpath(buffer, actualpath) == NULL)
        {
        printf("ERROR(bad path): %s\n", buffer);
        close(client_socket);
        return;
        }


        //read file and send its contents to client
        FILE *fp = fopen(actualpath, "r");
        if(fp == NULL)
        {
        printf("ERROR(open): %s\n", buffer);
        close(client_socket);
        return;
        }

        //read file contents and send them to client
        //should limit the client to certain files....!  maybe check if the path as a char or string containing some name then allow otherwise return 404 or message to deny
        while((bytes_read = fread(buffer, 1, BUFSIZE, fp)) > 0) {
            printf("Sending %zu bytes\n", bytes_read);
            write(client_socket, buffer, bytes_read);
        }
        close(client_socket);
        fclose(fp);
        printf("closing connection\n");
}