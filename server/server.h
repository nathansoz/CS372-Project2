/*
 Created by nathan on 8/12/17.
*/

#ifndef SERVER_SERVER_H
#define SERVER_SERVER_H

#include <stdint.h>
#include <string.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>

/*
 * Starts a server loop on a given port. The startup code uses:
 *  http://beej.us/guide/bgnet/output/html/singlepage/bgnet.html#simpleserver
 *
 */
void server_loop(const char* port);



#endif 
