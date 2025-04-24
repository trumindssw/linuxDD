#include "header.h"

int main()
{
	int udpSocket;
	char buffer[100];
	struct sockaddr_in serverAddr, clientAddr;
	socklen_t addr_size;

	/*Create UDP socket*/
	udpSocket = socket(PF_INET, SOCK_DGRAM, 0);

	/*Configure settings in address struct*/
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(PORT_NUM);
	serverAddr.sin_addr.s_addr = inet_addr("0.0.0.0");

//	memset(serverAddr.sin_zero, '\0', sizeof serverAddr.sin_zero);

	/*Bind socket with address struct*/
	bind(udpSocket, (struct sockaddr *) &serverAddr, sizeof(serverAddr));

	/*Initialize size variable to be used later on*/
	addr_size = sizeof(clientAddr);

	while(1)
	{
		recvfrom(udpSocket,buffer,100,0,(struct sockaddr *)&clientAddr, &addr_size);
                printf("the data from client is: %s\n",buffer);
		if(strcmp(buffer,"quit")==0)
		{
			break;
		}
	}
	return 0;
}
