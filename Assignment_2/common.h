#ifndef COMMON_H
#define COMMON_H
#define _POSIX_C_SOURCE 200809L

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <time.h>

#define SERVER_PORT      5000          
#define BUFSIZE          256           
#define INACTIVITY_SEC   (3 * 60)                   
#define GRACE_SEC        (2 * 60)      

#endif
