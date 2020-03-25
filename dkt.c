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

typedef struct {
	int node_key;
	char nodeIP[20];
	int succ_key;
	char succIP[20];
	int succ2;
	char succ2IP[20];
}stateInfo;


extern int errno;
stateInfo server;

int UserInput();
int CreateRing();


int main(int argc, char *argv[]){

	int fd, newfd, afd=0;
	ssize_t n;
	socklen_t addrlen;
	struct addrinfo hints, *res;
	struct sockaddr_in addr;
	char buffer[128], text[128];
	fd_set rfds;
	enum {idle, busy} state;
	int maxfd, counter;
	int i=0;


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
		FD_SET(fd,&rfds);	
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

				case busy:	write(newfd, "Busy\n", 5);
							close(newfd);
			}
		}

		if(state == busy && FD_ISSET(afd,&rfds)){
			if((n=read(afd,buffer,128))!=0){
				if(n==-1) exit(1);
				write(1, "received: ",10);
				write(1, buffer, n);
			}
			else{
				close(afd);
				state = idle;
				printf("Client disconnected\n");
			}
		}


		if(FD_ISSET(0,&rfds)){
			i=UserInput();
			if(i==1){
				printf("Invalid command\n");
			}
			else if(i==-1){
				printf("Closing dkt\n");
			}
			/*if(state == busy){
				n=write (afd,buffer,strlen(buffer));
				if(n==-1)/
					exit(1);
				printf("sent: %s", buffer);
			
				}*/
		}


	}
	exit(0);
}

int UserInput(){

	int n=-1, i=-1;
	char option[10]="\0", input[128]="\0";

	if(fgets(input, 128, stdin)==NULL){
		return 1;
	}

	if(!sscanf(input, " %s", option)){
		return 1;
	}

	if(strcmp(option, "new")==0){


		printf("%s selected\n", option);
		CreateRing();
		return 0;
	}

	if(strcmp(option, "entry")==0){

		printf("%s selected\n", option);
		return 0;
	}

	if(strcmp(option, "sentry")==0){

		printf("%s selected\n", option);
		return 0;
	}

	if(strcmp(option, "leave")==0){

		printf("%s selected\n", option);
		return 0;
	}

	if(strcmp(option, "show")==0){

		printf("%s selected\n", option);
		return 0;
	}

	if(strcmp(option, "find")==0){

		printf("%s selected\n", option);
		return 0;
	}

	if(strcmp(option, "exit")==0){

		printf("%s selected\n", option);
		return -1;
	}

	return 1;

}

int CreateRing(){
	/*

		coisas


	*/
	printf("Ring created\n");
	return 0;

}