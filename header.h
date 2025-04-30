// common.h


#ifndef COMMON_H 
#define COMMON_H
 
#include <stdio.h>                  //   Standard I/O functions (printf, scanf, etc.)
#include <stdlib.h>                 //   Standard library functions (malloc, free, exit, etc.)
#include <string.h>                 //   String manipulation functions (strcpy, memset, etc.)				 
#include <unistd.h>                 //    POSIX API (close, read, write, etc.)
#include <errno.h>                  //    Error number definitions and error handling
#include <time.h>                   //   Time-related functions (time, localtime, etc.)
#include <sys/socket.h>             //    Socket APIs (socket, bind, listen, etc.)
#include <sys/types.h>              //    Data types used in system calls
#include <netinet/in.h>             //    Structures for Internet domain addresses (sockaddr_in, etc)
#include <arpa/inet.h>              //   Functions for IP address conversion (inet_pton, inet_ntoa, etc.)


#define PORT 8080          //8080 is a common default for testing web servers and application servers.
			   // Port number is mandatory for server-client communication.
			   //  8080 is easy to use, especially for testing non-privileged apps.

#define MAX_BUF 1024       // Maximum buffer size in bytes



#define IDLE_TIME 180 // 3 minutes
#define DIE_TIME 120  // 2 minutes

#endif

