#include "headers.h"

void client_info(struct sockaddr_in *p) 
{
	puts("Client info...");
	printf("PORT Number: %hu\n", ntohs(p->sin_port));
	printf("IP address: %s\n", inet_ntoa(p->sin_addr));
}

int main(int argc, char **argv) 
{
	if (argc != 2) 
	{
		fprintf(stderr, "Usage: %s <port>\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	int sfd, cfd;
	char buff[MAX_BUF];
	struct sockaddr_in sa, ca;
	socklen_t csize = sizeof(ca);

	sfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sfd == -1) 
	{
		perror("socket");
		return -1;
	}

	sa.sin_family = AF_INET;
	sa.sin_port = htons(atoi(argv[1]));
	sa.sin_addr.s_addr = inet_addr("0.0.0.0");

	if (bind(sfd, (struct sockaddr *)&sa, sizeof(sa)) == -1) 
	{
		perror("bind");
		return -1;
	}

	if (listen(sfd, 1) == -1) 
	{
		perror("listen");
		return -1;
	}

	printf("Server waiting for client...\n");
	cfd = accept(sfd, (struct sockaddr *)&ca, &csize);
	if (cfd == -1) 
	{
		perror("accept");
		return -1;
	}

	printf("Client connected!\n");
	client_info(&ca);

	time_t last_activity = time(NULL);
	int last_printed_minute = 0;

	while (1) 
	{
		fd_set readfds;
		struct timeval timeout;
		FD_ZERO(&readfds);
		FD_SET(cfd, &readfds);

		timeout.tv_sec = 1;
		timeout.tv_usec = 0;

		int ready = select(cfd + 1, &readfds, NULL, NULL, &timeout);

		if (ready > 0 && FD_ISSET(cfd, &readfds)) 
		{
			int len = recv(cfd, buff, sizeof(buff) - 1, 0);
			if (len <= 0) 
			{
				printf("Client disconnected.\n");
				break;
			}
			buff[len] = '\0';

			// Remove newline
			char *nl = strpbrk(buff, "\r\n");
			if (nl) *nl = '\0';

			time_t now = time(NULL);
			last_activity = now;
			last_printed_minute = 0;

			printf("Received command: '%s'\n", buff);

			if (strcmp(buff, "time") == 0)
			{
				char tbuf[64];
				time_t t = time(NULL);
				strftime(tbuf, sizeof(tbuf), "%H:%M:%S", localtime(&t));
				send(cfd, tbuf, strlen(tbuf), 0);
			} 
			else if (strcmp(buff, "date") == 0)
			{
				char dbuf[64];
				time_t t = time(NULL);
				strftime(dbuf, sizeof(dbuf), "%Y-%m-%d", localtime(&t));
				send(cfd, dbuf, strlen(dbuf), 0);
			}
			else if (strcmp(buff, "exit") == 0) 
			{
				send(cfd, "Goodbye!", 8, 0);
				printf("Client requested exit.\n");
				break;
			}
			else
			{
				send(cfd, "Invalid command", 15, 0);
			}

		} 
		else 
		{
			// No activity
			time_t now = time(NULL);
			int inactive_secs = (int)difftime(now, last_activity);
			int inactive_mins = inactive_secs / 60;

			if (inactive_mins >= 5) 
			{
				printf("No message received for 5 minutes. Shutting down gracefully...\n");
				break;
			}

			if (inactive_mins > last_printed_minute)
			{
				last_printed_minute = inactive_mins;
				if (inactive_mins <= 3) 
				{
					printf("No message for %d minute%s...\n", inactive_mins, inactive_mins == 1 ? "" : "s");
				}
				if (inactive_mins == 3) 
				{
					printf("No message received for 3 minutes. Server will die in 2 minute(s)...\n");
				}
			}
		}
	}

	close(cfd);
	close(sfd);
	return 0;
}

