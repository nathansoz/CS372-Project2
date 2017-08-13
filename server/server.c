//
// Created by nathan on 8/12/17.
//

#include <unistd.h>
#include <stdbool.h>
#include "server.h"
#include <pthread.h>
#include <tgmath.h>

#include <dirent.h>

// get sockaddr, IPv4 or IPv6, from beej's guide
void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

/* This boilerplate code is from beej's guide to network programming */
int bind_server(const char* port)
{
    struct addrinfo hints, *servinfo, *p;
    int getaddrinfo_ret;
    int sockfd = -1;
    int yes = 1;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE; // use my IP

    if ((getaddrinfo_ret = getaddrinfo(NULL, port, &hints, &servinfo)) != 0)
    {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(getaddrinfo_ret));
        exit(1);
    }
    // loop through all the results and bind to the first we can
    for(p = servinfo; p != NULL; p = p->ai_next)
    {
        if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1)
        {
            perror("server: socket");
            continue;
        }

        if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1)
        {
            perror("setsockopt");
            exit(1);
        }

        if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1)
        {
            close(sockfd);
            perror("server: bind");
            continue;
        }

        break;
    }


    freeaddrinfo(servinfo);

    if (p == NULL)
    {
        fprintf(stderr, "server: failed to bind\n");
        exit(1);
    }

    if (listen(sockfd, 10 /*queue connections*/) == -1)
    {
        perror("listen");
        exit(1);
    }

    return sockfd;
}

int recv_int(int fd)
{
    ssize_t message_header_bytes = 0;
    int message_size = 0;
    while(message_header_bytes < sizeof(int))
    {

        ssize_t got = recv(fd, &message_size + message_header_bytes, sizeof(int) - message_header_bytes, 0);
        message_header_bytes =+ got;
    }

    message_size = ntohl((uint32_t)message_size);

    ssize_t message_bytes = 0;
    int message = 0;
    while(message_bytes < message_size)
    {
        ssize_t got = recv(fd, &message + message_bytes, message_size - (size_t)message_bytes, 0);
        message_bytes += got;
    }

    return ntohl((uint32_t)message);
}

char* recv_message(int fd)
{
    ssize_t message_header_bytes = 0;
    int message_size = 0;
    while(message_header_bytes < sizeof(int))
    {

        ssize_t got = recv(fd, &message_size + message_header_bytes, sizeof(int) - message_header_bytes, 0);
        message_header_bytes =+ got;
    }

    message_size = ntohl((uint32_t)message_size);

    ssize_t message_bytes = 0;
    char* message = malloc((size_t)message_size + 1);
    while(message_bytes < message_size)
    {
        ssize_t got = recv(fd, message + message_bytes, message_size - (size_t)message_bytes, 0);
        message_bytes += got;
    }
    message[message_size] = '\0';

    return message;
}

void send_message(int fd, char* message)
{
    size_t len = strlen(message);

    uint32_t message_size = htonl((uint32_t)len);

    size_t headerSize = sizeof(uint32_t);
    size_t headerRemaining = sizeof(uint32_t);
    do
    {
        headerRemaining -= send(fd, &message_size + (headerSize - headerRemaining), headerRemaining, 0);
    }
    while(headerRemaining > 0);


    uint32_t bodyRemaining = (uint32_t)len;
    do
    {
        bodyRemaining -= send(fd, message + (len - bodyRemaining), bodyRemaining, 0);

    }
    while(bodyRemaining > 0);
}

int open_client_connection(char* addr, int port)
{
    char charport[10];
    sprintf(charport, "%d", port);

    struct addrinfo hints, *res;
    int sockfd;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    int test = getaddrinfo(addr, charport, &hints, &res);

    if(test == -1)
        return -1;

    sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    int connected = connect(sockfd, res->ai_addr, res->ai_addrlen);

    if(connected != 0)
    {
        perror("Failed to connect");
        return -1;
    }

    freeaddrinfo(res);

    return sockfd;

}

struct thread_args
{
    int fd;
    char addr[INET6_ADDRSTRLEN];
};

void send_directory_listing(int client_fd)
{
    DIR *directory;
    struct dirent *dir;
    directory = opendir(".");
    if (directory)
    {
        while ((dir = readdir(directory)) != NULL)
        {
            if(strcmp(dir->d_name, ".") == 0 || strcmp(dir->d_name, "..") == 0)
            {
                continue;
            }

            //Only send files
            if(dir->d_type == DT_REG)
            {
                send_message(client_fd, dir->d_name);
            }
        }
        closedir(directory);
    }

    send_message(client_fd, "__DIR_LISTING_FINISHED__");
}

/* https://stackoverflow.com/questions/230062/whats-the-best-way-to-check-if-a-file-exists-in-c-cross-platform */
bool check_file_exists(char * file)
{
    if( access( file, F_OK ) != -1 )
    {
        return true;
    }

    return false;
}

/* https://stackoverflow.com/questions/174531/easiest-way-to-get-files-contents-in-c */
/* This DOES NOT handle where the file is too large to fit in a contiguous block of memory */
void send_file_binary(int fd, char * fileName)
{
    FILE *file  = fopen(fileName, "rb");

    fseek (file, 0, SEEK_END);
    long length = ftell(file);
    char* buffer = malloc(length);
    fseek(file, 0, SEEK_SET);
    fread (buffer, 1, length, file);
    fclose(file);

    uint32_t message_size = htonl((uint32_t)length);

    size_t headerSize = sizeof(uint32_t);
    size_t headerRemaining = sizeof(uint32_t);
    do
    {
        headerRemaining -= send(fd, &message_size + (headerSize - headerRemaining), headerRemaining, 0);
    }
    while(headerRemaining > 0);


    uint32_t bodyRemaining = (uint32_t)length;
    do
    {
        bodyRemaining -= send(fd, buffer + (length - bodyRemaining), bodyRemaining, 0);

    }
    while(bodyRemaining > 0);

    free(buffer);
}

void* command_loop(void* in)
{
    if(in == NULL)
    {
        return NULL;
    }

    int fd = ((struct thread_args*)in)->fd;
    char addr[INET6_ADDRSTRLEN];
    strcpy(addr, ((struct thread_args*)in)->addr);

    free(in);

    int data_port = recv_int(fd);
    printf("Will use data port %d\n", data_port);


    char* message = recv_message(fd);

    if(strcmp(message, "-l") == 0)
    {
        send_message(fd, "dir");
        int client_fd = open_client_connection(addr, data_port);

        if(client_fd == -1)
        {
            return NULL;
        }

        send_directory_listing(client_fd);
        close(client_fd);
    }
    else if(strlen(message) > 3 && message[0] == '-' && message[1] == 'g' && message[2] == ' ')
    {
        size_t tocopy = strlen(message) - 2; //first few bytes, but make sure to grab the terminator
        char buffer[tocopy];
        memcpy(buffer, message + 3, tocopy);

        if(!check_file_exists(buffer))
        {
            send_message(fd, "File doesn't exist!");
        }
        else
        {
            send_message(fd, "transfer");
            int client_fd = open_client_connection(addr, data_port);
            send_file_binary(client_fd, buffer);
            close(client_fd);
        }
    }
    else
    {
        send_message(fd, "Invalid command!");
    }

    free(message);
    return NULL;
}



void server_loop(const char* port)
{
    int socket = bind_server(port);

    while(true)
    {
        char s[INET6_ADDRSTRLEN];
        struct sockaddr_storage their_addr;
        socklen_t sin_size = sizeof their_addr;
        int new_fd = accept(socket, (struct sockaddr *)&their_addr, &sin_size);
        if (new_fd == -1)
        {
            perror("accept");
            break;
        }

        inet_ntop(their_addr.ss_family, get_in_addr((struct sockaddr *)&their_addr), s, sizeof s);
        printf("server: got connection from %s\n", s);

        pthread_t thread;

        struct thread_args* arg = malloc(sizeof(struct thread_args));
        arg->fd = new_fd;
        strcpy(arg->addr, s);

        pthread_create(&thread, NULL, command_loop, (void*)arg);
    }
}

