/*
 * main.c
 *  This file validates arguments and dispatches to the main server
 */

#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>

#include "server.h"

#define PORT_MAX 65535

/*
 * Validates that a port argument is actually a port
 */
bool validate_port(char* str)
{
    char* end;
    long value = strtol(str, &end, 10 /* base 10 */);

    if(strcmp(end, "") != 0)
    {
        return false;
    }

    if(value <= 0 || value > PORT_MAX)
    {
        return false;
    }

    return true;
}

void print_usage(bool error)
{
    if(error)
    {
        printf("Error! Invalid usage.\n\n");
    }

    printf("Usage:\n");
    printf("\tfserve <port>\n");

    if(error)
    {
        exit(1);
    }
}

int main(int argc, char** argv)
{
    if(argc != 2)
    {
        print_usage(true);
    }

    if(strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0)
    {
        print_usage(false);
        exit(0);
    }

    if(!validate_port(argv[1]))
    {
        printf("Error! Invalid port specified!\n");
        exit(1);
    }

    printf("Welcome to fserver!\n");
    printf("User specified port is: %s\n", argv[1]);
    printf("Starting server...\n");

    server_loop(argv[1]);
    return 0;
}
