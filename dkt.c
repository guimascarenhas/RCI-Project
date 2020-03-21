#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#define max(A,B) ((A)>=(B)?(A):(B))
#define AAA printf("aqui\n");

extern int errno;

int main(int argc, char *argv[]){

	int fd, newfd, afd=0;
	ssize_t n;
	socklen_t addrlen;
	struct addrinfo hints, *res;
	struct sockaddr_in addr;
	char buffer[128];
	fd_set rfds;
	enum {idle, busy} state;
	int maxfd=0, counter;

	if(argc < 3){
		printf("Missing arguments when calling dkt\n");
		exit(1);
	}


	//create socket
	fd=socket(AF_INET, SOCK_STREAM, 0);
	if(fd==-1){
		printf("Could not create socket\n");
		exit(1);
	}

	memset(&hints, 0, sizeof(hints));
	hints.ai_family=AF_INET;
	hints.ai_socktype=SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	getaddrinfo(NULL, argv[2], &hints, &res);
	if(errno!= 0){
		fprintf(stderr, "error %s\n", gai_strerror(errno));
		exit(1);
	}

	errno=bind(fd, res->ai_addr, res->ai_addrlen);
	if(errno==-1){
		printf("Could not bind socket\n");
		exit(1);
	}
	printf("Socket created and binded\n");

	if(listen(fd,5)==-1){
		perror("Error ");
		exit(1);
	}
	printf("listening\n");

	state = idle;

	while(1){
		FD_ZERO(&rfds);
		FD_SET(0,&rfds);	
		if(maxfd==0) FD_SET(fd,&rfds);	//skips the first iteration of the loop
		maxfd=fd;
		if(state==busy)	{
			FD_SET(afd,&rfds);
			maxfd=max(maxfd,afd);
		}

		counter=select(maxfd+1, &rfds, (fd_set*)NULL, (fd_set*)NULL, (struct timeval *)NULL);
		if(counter<=0) exit(1);

		if(FD_ISSET(fd,&rfds)){

			addrlen=sizeof(addr);
			if((newfd=accept(fd, (struct sockaddr*)&addr, &addrlen))==-1) exit(1);
			switch(state){
				case idle:	afd=newfd;
							state=busy;
							printf("Client connected\n");
							break;

				case busy:	close(newfd);
			}
		}

		if(FD_ISSET(afd,&rfds)){
			if((n=read(afd,buffer,128))!=0){
				if(n==-1) exit(1);
				write(1, "received: ",10);
				write(1, buffer, n);
			}
			else{
				close(afd);
				state = idle;
			}
		}

		if(FD_ISSET(0,&rfds)){
			fgets(buffer, 128, stdin);
			printf("inserido: %s", buffer);
		}


	}
	exit(0);
}