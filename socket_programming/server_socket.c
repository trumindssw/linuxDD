#include "headers.h"

int main(int argc, char *argv[])
{
	char buff[100];
	int fd, client_fd;
	struct sockaddr_in server_addr, client_addr;
	socklen_t addr_len = sizeof(client_addr);

	// Create TCP socket
	fd = socket(AF_INET, SOCK_STREAM, 0);
	if (fd < 0)
	{
		printf("Socket creation failed\n");
		return -1;
	}
	printf("Server socket created\n");

	// Set server address
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(atoi(argv[1]));
	server_addr.sin_addr.s_addr = INADDR_ANY;

	// Bind socket
	if (bind(fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
	{
		printf("Binding failed\n");
		close(fd);
		return -1;
	}
	printf("Binding successful\n");

	// Listen for connections
	if (listen(fd, 3) < 0)
	{
		printf("Listening failed\n");
		close(fd);
		return -1;
	}
	printf("Server is listening\n");

	// Accept connection
	client_fd = accept(fd, (struct sockaddr *)&client_addr, &addr_len);
	if (client_fd < 0)
	{
		printf("Connection not accepted\n");
		close(fd);
		return -1;
	}
	printf("Client connected\n");

	// Communicate with client
	while (1)
	{
		memset(buff, 0, sizeof(buff));
		recv(client_fd, buff, sizeof(buff), 0);
		printf("Client: %s\n", buff);

		if (strcmp(buff, "exit") == 0)
		{
			printf("Client sent 'exit'. Closing server.\n");
			break;
		}
		else if (strcmp(buff, "time") == 0)
		{
			time_t now = time(NULL);
			struct tm *tm_info = localtime(&now);
			char time_str[20];
			strftime(time_str, sizeof(time_str), "%H:%M:%S", tm_info); // <-- Only time
			send(client_fd, time_str, strlen(time_str), 0);
		}
		else if (strcmp(buff, "date") == 0)
		{
			time_t now = time(NULL);
			struct tm *tm_info = localtime(&now);
			char date_str[20];
			strftime(date_str, sizeof(date_str), "%Y-%m-%d", tm_info); // <-- Only date
			send(client_fd, date_str, strlen(date_str), 0);
		}

	}

	// Close sockets
	close(client_fd);
	close(fd);

	return 0;
}

