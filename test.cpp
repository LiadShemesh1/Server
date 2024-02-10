#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <stdbool.h>
#include <limits.h>
#include <iostream>
#include <netdb.h>
#include <pthread.h>

#include <errno.h>
#include <signal.h>

#define SERVERPORT "8989"  // the port users will be connecting to
#define BUFSIZE 4096  //todo why this size???
#define SOCKETERROR (-1)
#define SERVER_BACKLOG 100 // how many pending connections queue will hold, kind of useless...


// typedef allows us to use these, with a more meaningful name
typedef struct sockaddr SA;
typedef struct sockaddr_storage SAS; 
typedef struct addrinfo ADDINFO;

// a struct used to pass argument to thread.
struct thread_client_address{
        int _client_socket;
        SAS _their_addr;
};


// here for linking (or compiling), actual implimintation is down.
void *handle_connection(void* arg);
/* chack is a general "catch error" function. most of the build in socket function return the same values for 
seccses and failing (-1). */ 
int check(int exp, const char *msg);

void *get_in_addr(struct sockaddr *sa);
// signalHandler will handle signals such as unexpected disconnect socket.
void signalHandler(int signal);

int main(int argc , char **argv)
{

        int server_socket, client_socket; // client_socket not sure yet
        socklen_t addr_size;
        SAS their_addr;
        ADDINFO hints, *res;
        pthread_mutex_t mutex; // maybe should be in the while loop...??
        // signal handler using sigaction:
        struct sigaction sa;
        sa.sa_handler = signalHandler;
        // Set up sigaction handler for SIGPIPE == Client disconnected unexpectedly.
        sigaction(SIGPIPE, &sa, NULL);

        // first, load up address structs with getaddrinfo():
        memset(&hints, 0, sizeof(hints)); // zero out hints; 
        //memset ensures that the structure is properly initialized before setting specific fields, 
        //reducing the chances of encountering bugs related to uninitialized memory.
        hints.ai_family = AF_INET;       // use IPv4
        hints.ai_socktype = SOCK_STREAM; // TCP
        hints.ai_flags = AI_PASSIVE;     // fill in my IP for me


        // When successful, getaddrinfo() returns 0 and a pointer to a linked list of one or more
        // addrinfo structures through the res argument.
        getaddrinfo(NULL, SERVERPORT, &hints, &res);
        
        // craete the socket using the *res which points to hints with the parameters above:
        check((server_socket = socket(res->ai_family, res->ai_socktype, res->ai_protocol)), 
                "Failed to create socket");
         
        // prevents cases of "address already in use":
        int yes=1; 
        if(setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &yes, sizeof(yes))) {
                perror("setsockopt");
                exit(EXIT_FAILURE);
        }

        // bind it to the port we passed in to getaddrinfo():
        check(bind(server_socket, res->ai_addr, res->ai_addrlen), 
                "Bind Failed");

        check(listen(server_socket, SERVER_BACKLOG),
                "Listen Failed");

        freeaddrinfo(res); // free the linked-list
        
        while(true)
        {       
                std::cout << "waiting for connections... \n" << std::endl;
                addr_size = sizeof(their_addr); // chaeck
                
                check(client_socket = 
                        accept(server_socket, (SA*)&their_addr, &addr_size),
                        "Accept Faild");
                std::cout << "Connected \n" << std::endl;
                
                pthread_t ptid; 
                // start mutex:
                //pthread_mutex_lock(&mutex);
                thread_client_address *thread_args = new thread_client_address({client_socket, their_addr});

                //create a thread to handle the connection passing multipule argumentsusing a struct:
                pthread_create(&ptid, NULL, handle_connection, thread_args);

                //End mutex:
                //pthread_mutex_unlock(&mutex);

                // waits for the thread to finish, and our case terutn NULL from the handle_connection function
                //should be outside of mutex....?
                //pthread_join(ptid, nullptr);

        } // while
        return 0;
}

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(SA *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}


// a check function that takes advantage of the fact that most socket related function fail behavior are the same
// if an error occured it prints the error. 
int check(int exp, const char *msg)
{       
        if(exp == SOCKETERROR){
                perror(msg);
                exit(1);
        }
        return exp;
}

void signalHandler(int signal) {
    if (signal == SIGPIPE) {
        // Handle broken pipe error (code=141), unexpected client disconnected.
        std::cerr << "Client disconnected unexpectedly." << std::endl;
    }
    // write to a log file the error.
}


void *handle_connection(void* arg)
{
        // unpack my args from the struct, explicit conversiob from void* to my struct pointer.
        thread_client_address *my_args = (thread_client_address*)arg;
        int client_socket = my_args->_client_socket;
        SAS their_addr = my_args->_their_addr;
        delete my_args; // not needed anymore.

        char buffer[BUFSIZE]; // The size of each buffer we will read
        size_t bytes_read;
        int msgsize = 0;
        char actualpath[PATH_MAX + 1];  // +1 is for the null termination character.
        char s[INET6_ADDRSTRLEN];  // taking the maximum size of an address

        // prints the client address:
        inet_ntop(their_addr.ss_family,
                get_in_addr((SA*)&their_addr),
                s, sizeof(s));
        
        std::cout << "server: got connection from " << s << '\n' << std::endl;

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

        std::cout << "REQUEST: " << buffer << std::endl;

        fflush(stdout); // makes sure that all the recived data was written.
        // fflush(stdout) checks if there are any data in the buffer that should be written and if so,
        // the underlying syscall is used to write the data to the OS.
        /*
        fflush() is typically used for output stream only. Its purpose is to clear (or flush)
        the output buffer and move the buffered data to console (in case of stdout) 
        or disk (in case of file output stream)
        */

        //validity check, checks if the file exist as well
        //need to replay with an http error 404 if happened todo
        if(realpath(buffer, actualpath) == NULL)
        {
        std::cout << "ERROR: Bad path " << buffer << std::endl;
        close(client_socket);
        return NULL;
        }


        //read file and send its contents to client
        FILE *fp = fopen(actualpath, "r");
        if(fp == NULL)
        {
        std::cout << "ERROR(open): " << buffer << std::endl;
        close(client_socket);
        return NULL;
        }

        sleep(1);  // test for a case of a slow disk or remote resver.

        //read file contents and send them to client
        //should limit the client to certain files....!  maybe check if the path as a char or string containing some name then allow otherwise return 404 or message to deny

        while((bytes_read = fread(buffer, 1, BUFSIZE, fp)) > 0) {
                std::cout << "sending " << bytes_read << "bytes" << std::endl;
                
                // if write failed, most likely that a SIGPIPE (Client disconnected unexpectedly) occured so we done with this client.
                if(write(client_socket, buffer, bytes_read) < 0){
                        close(client_socket);
                        fclose(fp);
                        return NULL;
                } 

        }

        close(client_socket);
        fclose(fp);
        std::cout << "closing connection" << std::endl;
        return NULL;
}