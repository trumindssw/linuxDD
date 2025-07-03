#include "common.h"

static void errormsg(const char *msg) { 
    perror(msg); 
    exit(EXIT_FAILURE); 
}

static void send_string(int fd, const char *s)
{
    size_t len = strlen(s);
    if (write(fd, s, len) != (ssize_t)len) errormsg("write");
}

static void format_time(char *out, size_t n)
{
    // get the current time
    time_t now = time(NULL); 
    struct tm *tm = localtime(&now);
    strftime(out, n, "%H:%M:%S\n", tm);
}

static void format_date(char *out, size_t n)
{
    time_t now = time(NULL); 
    struct tm *tm = localtime(&now);
    strftime(out, n, "%d-%m-%Y\n", tm);
}


int main() {

    // creating the socket IPv4
    int srv = socket(AF_INET, SOCK_STREAM, 0);
    // checking we got the socket or not
    if (srv < 0) errormsg("socket");

    // it is use for reuse PORT , Address
    int opt = 1;
    if (setsockopt(srv, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
        errormsg("setsockopt");

    // setting up the ip and port
    struct sockaddr_in addr = {
        .sin_family = AF_INET,
        .sin_addr.s_addr = htonl(INADDR_ANY),
        .sin_port = htons(SERVER_PORT)
    };

    // now socket is created we have to bind the socket with ip + port
    // if it gives 0 it means success otherwise throw error
    if (bind(srv, (struct sockaddr *)&addr, sizeof(addr)) < 0) errormsg("bind");

    // now socket is attached to ip+port, start listening
    int listening = listen(srv,1);
    // if listening does not give 0 (success) throw error
    if (listening < 0) errormsg("listen");

    // print on screen to show the user that server is listening
    printf("Server listening on port %d …\n", SERVER_PORT);

    // accepting request from the client
    int client_req = accept(srv, NULL, NULL);
    // checking the connection is established or not
    if (client_req < 0) errormsg("accept");
    printf("Client connected. %d\n", client_req);

    // now we got connected with client we have to perform the tasks
    // Task1 -> reply to the client_req
    // Task2 -> check for inactivity

    time_t last_activity = time(NULL);
    time_t warned_at     = 0;
    enum { ACTIVE, WARNED } state = ACTIVE;

    for (;;)
    {
        fd_set rfds; 
        FD_ZERO(&rfds); 
        FD_SET(client_req, &rfds);
        struct timeval tv = { .tv_sec = 1, .tv_usec = 0 };
        (void)select(client_req + 1, &rfds, NULL, NULL, &tv);

        // getting current time
        time_t now = time(NULL);

        // Task 1 -> if we got something from client we will read and reply
        if (FD_ISSET(client_req, &rfds))
        {
            char client_msg[BUFSIZE] = {0};
            // reading from the client into the buffer
            ssize_t n = read(client_req, client_msg, sizeof(client_msg) - 1);
            if (n <= 0) { 
                puts("Client closed connection."); 
                break; 
            }

            last_activity = now; 
            state = ACTIVE; 
            warned_at = 0;

            if (strncasecmp(client_msg, "time", 4) == 0) {
                char timebuffer[64]; 
                // formatting time into readable format
                format_time(timebuffer, sizeof timebuffer);
                send_string(client_req, timebuffer);
            } 
            else if (strncasecmp(client_msg, "date", 4) == 0) {
                char datebuffer[64]; 
                // formatting date into readable format
                format_date(datebuffer, sizeof datebuffer);
                send_string(client_req, datebuffer);
            } 
            else {
                send_string(client_req, "Unknown command\n");
            }
        }

        // Task 2 -> check for inactivity
        if (state == ACTIVE && now - last_activity >= INACTIVITY_SEC) {
            printf("No message for %d min; will errormsg in 2 min …\n", INACTIVITY_SEC / 60);
            state = WARNED;
            warned_at = now;
        }

        if (state == WARNED && now - warned_at >= GRACE_SEC) {
            puts("Grace period elapsed. Shutting down.");
            break;
        }
    }

    // closing the sockets
    close(client_req); 
    close(srv);

    return 0;
}
