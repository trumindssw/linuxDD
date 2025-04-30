// common.h
 
 
#ifndef COMMON_H
#define COMMON_H
 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
 
#define PORT 8080          //8080 is a common default for testing web servers and application servers.
                           // Port number is mandatory for server-client communication.
                           //  8080 is easy to use, especially for testing non-privileged apps.
 
#define MAX_BUF 1024       
 
 
#define IDLE_TIME 180 // 3 minutes
#define DIE_TIME 120  // 2 minutes
 
#endif
