#include "headers.h"

int main(int argc,char *argv[])
{
	char buff[100];
	int msg[4];
	char wrong_cmd[100];
	if(argc!=2)
	{
		puts("correct usage of execution is: ./exe_file server_ip");
		return 0;
	}

	//creating client socket

	int cfd=socket(AF_INET,SOCK_STREAM,0);

	if(cfd<0)
	{
		perror("socket");
		return 0;
	}
        //client socket created successfully

	//("Binding & Connecting: ");
	//("Mapping the file descripted with Server's IP & Server Port ( Remote Socket Address)");
	//("Also mapping With Client's IP & Ephemeral Port (Local Socket Address)");
	//("Initiating 3-Way Handshake for Connection Establishement With Server...");
	struct sockaddr_in addr;
	int len=sizeof(addr);
	addr.sin_family=AF_INET;
	addr.sin_port=htons(PORT_NUM);
	addr.sin_addr.s_addr=inet_addr(argv[1]);
	if(connect(cfd,(struct sockaddr *)&addr,len)<0)
	{
		perror("connect");
		return 0;
	}
	//Connection Success With Server


	while(1)
	{
	puts("Enter some commands like date,time to send to server or enter quit to quit the communication.");

	fgets(buff,100,stdin);
	buff[strlen(buff)-1]=0;

	send(cfd,buff,strlen(buff)+1,0);
	puts("Message Sent");

	if(strcmp(buff,"quit")==0)
	{
		break;
	}
        
//	recv(cfd,msg,100,0);

        if(strcmp(buff,"time")==0)
	{
		recv(cfd,msg,100,0);
	printf("the time is (hrs:min:sec)= %d:%d:%d\n",msg[0],msg[1],msg[2]);
        }
	else if(strcmp(buff,"date")==0)
        {
		recv(cfd,msg,100,0);
        printf("the date is (day:month:year)= %d:%d:%d\n",msg[0],msg[1],msg[2]);
        }
	else
	{
          recv(cfd,wrong_cmd,100,0);
	  printf("%s:\n",wrong_cmd);
	}
	bzero(msg,4);   //clearing the msg buffer
	}
	close(cfd);
}


