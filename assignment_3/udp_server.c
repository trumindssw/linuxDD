#include "ipc_common.h"

int main() {
    struct sockaddr_in addr;

    // Create UDP socket
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        perror("socket creation failed");
        exit(1);
    }
    addr.sin_family = AF_INET;
    addr.sin_port = htons(9000);
    addr.sin_addr.s_addr = INADDR_ANY;

    bind(sock, (struct sockaddr*)&addr, sizeof(addr));

    char buffer[MAX_INPUT];
    printf("UDP server is running on port 9000, waiting for messages...\n");

    while (1) {
        socklen_t len = sizeof(addr);
        int n = recvfrom(sock, buffer, sizeof(buffer)-1, 0,
                         (struct sockaddr*)&addr, &len);
        if (n < 0) {
            perror("recvfrom failed");
            continue;
        }
        buffer[n] = '\0';
        printf("The content in shared memory received from the reader: %s\n", buffer);
    }
    close(sock);
    return 0;
}
