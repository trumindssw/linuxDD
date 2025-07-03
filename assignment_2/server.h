#ifndef SERVER_H
#define SERVER_H

void handle_client(int client_sock);
void graceful_exit(int signum);

#endif
