#include "header.h" 

int main() {
	// Create a UDP socket
	int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	if (sockfd < 0) {
		perror("Socket error");  // Print error if socket creation fails
		exit(1);
	}

	struct sockaddr_in servaddr, cliaddr;  // Structures to hold server and client addresses

	// Zero out the server address structure
	memset(&servaddr, 0, sizeof(servaddr));

	// Assign address family, IP address, and port
	servaddr.sin_family = AF_INET;          // IPv4
	servaddr.sin_port = htons(PORT);        // Port number (macro should be defined in header)
	servaddr.sin_addr.s_addr = INADDR_ANY;  // Accept connections from any IP address

	// Bind the socket to the specified IP and port
	if (bind(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) < 0) {
		perror("Bind failed");  // Print error if bind fails
		close(sockfd);
		exit(1);
	}

	char buffer[1024];             // Buffer to receive data
	socklen_t len = sizeof(cliaddr);  // Length of client address

	printf("UDP server ready on port %d...\n", PORT);  // Indicate server is ready

	// Infinite loop to receive messages from clients
	while (1) {
		// Receive data from a client
		ssize_t n = recvfrom(sockfd, buffer, sizeof(buffer), 0,
				(struct sockaddr*)&cliaddr, &len);

		if (n > 0) {
			buffer[n] = '\0';  // Null-terminate the received string
			printf("Server received: %s\n", buffer);  // Display the received message

			if (strcmp(buffer, "exit") == 0)
				break;  // Exit if message is "exit"
		}
	}

	close(sockfd);  // Close the socket before exiting
	return 0;
}

