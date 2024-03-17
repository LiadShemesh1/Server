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
#include <fcntl.h>
#include <sys/stat.h>
#include <errno.h>
#include <signal.h>
#include <string>
#include <time.h>

#define SERVERPORT "8989"  // the port users will be connecting to
#define BUFSIZE 4096
#define SOCKETERROR (-1)
#define SERVER_BACKLOG 100 // how many pending connections queue will hold, kind of useless...
#define TimeLen 21 // maximum length of time in the selected format with a white space at the end.


// typedef allows us to use these, with a more meaningful name
typedef struct sockaddr SA;
typedef struct sockaddr_storage SAS; 
typedef struct addrinfo ADDINFO;

// a struct used to pass argument to thread.
typedef struct {
        int _client_socket;
        SAS _their_addr;
} client_socket_and_address;

// maybe declere the lock close to the function as it only used there?
pthread_mutex_t time_lock = PTHREAD_MUTEX_INITIALIZER; //lock for the timeThreadSafe function.


// here for linking (or compiling), actual implimintation is down.
void *handle_connection(void* arg);
int check(int exp, const char *msg);
void *get_in_addr(struct sockaddr *sa);
void signalHandler(int signal);
void timeThreadSafe(char *msg, int size);

int main(int argc , char **argv)
{
        int server_socket, client_socket, gai;
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
        /*memset ensures that the structure is properly initialized before setting specific fields, 
        reducing the chances of encountering bugs related to uninitialized memory.*/
        hints.ai_family = AF_INET;       // use IPv4
        hints.ai_socktype = SOCK_STREAM; // TCP
        hints.ai_flags = AI_PASSIVE;     // fill in my IP for me


        /* When successful, getaddrinfo() returns 0 and a pointer to a linked list of one or more
        addrinfo structures through the res argument.*/
        if (gai = getaddrinfo(NULL, SERVERPORT, &hints, &res) != 0) {
                std::cerr << "getaddrinfo error: " << gai_strerror(gai) << "\n";
                exit(EXIT_FAILURE);
        }
        
        
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
                std::cout << "waiting for connections..." << std::endl;
                addr_size = sizeof(their_addr); // chaeck
                
                check(client_socket = 
                        accept(server_socket, (SA*)&their_addr, &addr_size),
                        "Accept Faild");
                std::cout << "Connected \n" << std::endl;
                
                pthread_t ptid; 
                // start mutex:
                //pthread_mutex_lock(&mutex);
                client_socket_and_address *thread_args = new client_socket_and_address({client_socket, their_addr});

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


/* a check function that takes advantage of the fact that most socket related function fail behavior are the same
if an error occured it prints the error. */
int check(int exp, const char *msg)
{       
        if(exp == SOCKETERROR){
                perror(msg);
                exit(1);
        }
        return exp;
}

void timeThreadSafe(char *msg, int size){
        pthread_mutex_lock(&time_lock);
        time_t t = time(NULL);
        struct tm date;
        if(gmtime_r(&t, &date) == NULL) {t = 0;} //making sure we have some time to write.
        localtime_r(&t, &date); // converting to local mechine timezone.

        strftime(msg, size, "%d-%m-%Y %H:%M:%S ", &date);
        pthread_mutex_unlock(&time_lock);
}

// signalHandler will handle signals such as unexpected disconnect socket.
// TODO: send the error with the client ip\mac, so we can act against it if the behaviour happend again
// its not an option to pass 2 args to the function, stackoverflow suggests to use global var.
void signalHandler(int signal) {
        char msg1[70];

        timeThreadSafe(msg1, TimeLen); // writing the current time to the begining of msg1.

        // lock() - maybe lock the from the beginning?
        if (signal == SIGPIPE){
        sprintf(msg1 + strlen(msg1), "Client disconnected unexpectedly.\n");
        // Handle broken pipe error (code=141), unexpected client disconnected.
        }

        int file = open("log.txt", O_WRONLY | O_CREAT | O_APPEND, S_IRUSR | S_IWUSR); //O_APPEND - withut O_APPEND it will always write to the begining of the file (overwriting the existing text) 
        write(file, msg1, strlen(msg1));
        close(file);
        // write to the terminal as well:
        write(STDOUT_FILENO, msg1, strlen(msg1));

        // unlock()
}


void *handle_connection(void* arg)
{
        // unpack my args from the struct, explicit conversiob from void* to my struct pointer.
        client_socket_and_address *my_args = (client_socket_and_address*)arg;
        int client_socket = my_args->_client_socket;
        SAS their_addr = my_args->_their_addr;
        delete my_args; // not needed anymore.

        char buffer[BUFSIZE]; // The size of each buffer we will read
        size_t bytes_read;
        int msgsize = 0;
        char actualpath[PATH_MAX + 1];  // +1 is for the null termination character.
        char s[INET_ADDRSTRLEN];  // space to hold the IPv4 string

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

        //validity check, checks if the file exist as well, make the path 

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
        //todo should limit the client to certain files....!  maybe check if the path as a char or string containing some name then allow otherwise return 404 or message to deny
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