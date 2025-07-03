#include "common.h"


static void errormsg(const char *m) { perror(m); exit(EXIT_FAILURE); }

int main(int argc, char *argv[])
{
    const char *host = (argc > 1) ? argv[1] : "127.0.0.1";

    // establishing the socket and throwing error if we dont got it
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) errormsg("socket");

    // setting up the Port Ipv4
    struct sockaddr_in srv = {
        .sin_family = AF_INET,
        .sin_port = htons(SERVER_PORT)
    };

    
    if (inet_pton(AF_INET, host, &srv.sin_addr) != 1) errormsg("inet_pton");

    // connecting to the server
    if (connect(sock, (struct sockaddr *)&srv, sizeof(srv)) < 0) errormsg("connect");

    // if it returns 0 then successfully connected
    puts("Connected Successfully to server. \nType commands (Time / date) or Ctrl‑D to quit.");


    char server_reply[BUFSIZE];
    while (fgets(server_reply, sizeof server_reply, stdin))
    {
        size_t len = strlen(server_reply);
        if (write(sock, server_reply, len) != (ssize_t)len) errormsg("write");

        ssize_t n = read(sock, server_reply, sizeof server_reply - 1);
        if (n <= 0) { 
            puts("Server closed connection."); 
            break; }
        server_reply[n] = '\0';
        printf(">> %s", server_reply);
    }

    close(sock);
    return 0;
}
