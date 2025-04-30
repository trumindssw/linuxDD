#include "headers.h"


int sfd,cfd;
static int time_out=0;

void gettime(struct tm *local,int cfd)
{
    int real_time[4]={local->tm_hour,local->tm_min,local->tm_sec};
    send(cfd,real_time,sizeof(real_time),0);
}

void getdate(struct tm *local,int cfd)
{
	int date[4]={local->tm_mday,((local->tm_mon)),((local->tm_year)+1900)};
	send(cfd,date,sizeof(date),0);
}

void handle_timeout(int sig) {
	time_out++;
	if(time_out==1)
	{
                     printf("\nNo message from client for 1 minute. within another 1 minute if client didnot respond server will shutdown.\n");
                     alarm(TIME_OUT);
	}
	else
	{
        	puts("now server got shut down");
                close(sfd);
                close(cfd);
                exit(0);
	}
}


int main()
{
	char buffer[100];
         
	//creating a socket
        sfd=socket(AF_INET,SOCK_STREAM,0);
        if(sfd<0)
	{
		perror("socket");
		return 0;
	}

        //server socket creation is completed
	
	signal(14,handle_timeout);

	struct sockaddr_in saddr;
	socklen_t slen=sizeof(saddr);
	saddr.sin_family=AF_INET;
	saddr.sin_port=htons(PORT_NUM);
	saddr.sin_addr.s_addr=inet_addr("0.0.0.0");  //can be an interface to any device


	//binding with the socket i.e assigning local address to the socket(ip_address + portnum)
	if(bind(sfd,(const struct sockaddr *)&saddr,slen)<0)
	{
		perror("bind");
		return 0;
	}
        //bind success

	//creating a connection queue size for 1 member
	if(listen(sfd,1)<0)
	{
		perror("listen");
		return 0;
	}

	struct sockaddr_in caddr;
        socklen_t clen=sizeof(caddr);
	/*("waiting for to accept the 3 way handshaking between client and server ans establishing a new socket connection between client and server"*/

	 cfd=accept(sfd, (struct sockaddr *)&caddr, &clen);

	 if(cfd<0)
	 {
		 perror("accept");
		 close(sfd);
		 return 0;
	 }
	 //accepting the connection request from client is accepted

	 //resetting the alarm to 60 seconds
          alarm(TIME_OUT);

	 while(1)
	 {

	 puts("waiting for the requested command from client:");

	 recv(cfd,buffer,100,0);
	 time_out=0;   //making time_ote value to zero after msg received
		       //
	 alarm(TIME_OUT);

	 printf("the received command from client is: %s\n",buffer);

	 time_t t;
        struct tm *local;
        t = time(NULL);  // Get current time
        local=localtime(&t);

	 if(strcmp(buffer,"time")==0)
	 {
		 gettime(local,cfd);
	 }
	 else if(strcmp(buffer,"date")==0)
	 {
		 getdate(local,cfd);
	 }
	 else if(strcmp(buffer,"quit")==0)
	 {
		 break;
	 }
	 else
	 {
		 send(cfd,"given command is not a proper command.Please provide proper commands like date,time or quit",100,0);
	 }
	 }
}

