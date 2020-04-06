#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include "headers.h"
#define max(A,B) ((A)>=(B)?(A):(B))
#define AAA printf("aqui\n")
#define MAX_KEY 100 //=N do enunciado 


extern int errno;
stateInfo server;
int flag_new; // flag_new = 0 if new has not been called and server is not connected
int pred_fd, succ_fd;
int maxfd;


int main(int argc, char *argv[]){

	int fd, newfd, afd=0, counter,i=0;
	ssize_t n;
	socklen_t addrlen;
	struct addrinfo hints, *res;
	struct sockaddr_in addr;
	char buffer[128];
	fd_set rfds;
	enum {idle, busy} state;

	flag_new=0;
	pred_fd=-1;	// fd is not in use 
	succ_fd=-1;

	if(argc < 3){
		printf("Missing arguments when calling dkt\n");
		exit(1);
	}

	//stores host info 
	sprintf(server.nodeIP,"%s", argv[1]);
	server.nodeTCP= atoi(argv[2]);
	server.node_key=-1;

	//create socket for listening
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
	maxfd=fd;

	while(1){
		FD_ZERO(&rfds);
		FD_SET(0,&rfds);	
		FD_SET(fd,&rfds);	
		if(pred_fd>0) FD_SET(pred_fd,&rfds);
		if(succ_fd>0) FD_SET(succ_fd,&rfds);
	
		if(state==busy)	{
			FD_SET(afd,&rfds);
			maxfd=max(maxfd,afd);
		}
		counter=select(maxfd+1, &rfds, (fd_set*)NULL, (fd_set*)NULL, (struct timeval *)NULL);
		if(counter<=0) exit(1);

		if(flag_new == 1 && FD_ISSET(fd,&rfds)){ //aceitar ligações
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
				if(messageHandler(afd,buffer)){
					state=idle;
				}
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
		}

		if(succ_fd>0 && FD_ISSET(succ_fd,&rfds)){
			if((n=read(succ_fd,buffer,128))!=0){
				if(n==-1) exit(1);
				write(1, "received from succ: ",20);
				write(1, buffer, n);
				succMessageHandler(buffer);
				maxfd=max(maxfd, succ_fd);
			}
		}


	}
	exit(0);
}

int dist(int k, int l){
	int d=0;

	if(k>l){
		d= MAX_KEY-(k-l);
	}
	else{
		d=l-k;
	}

	return d;
}

/*float dN(int k, int l){
	return ((l-k)/(float)MAX_KEY); // tirei o módulo, pq senão as comparações não fazem sentido
}*/

void succMessageHandler(char *buffer){

	char option[10];


	if(!sscanf(buffer, " %s", option)) return;

	if(strcmp(option,"SUCC")==0){
		sscanf(buffer,"%s %d %s %d", option, &server.succ2_key,server.succ2IP,&server.succ2TCP);
	}
	else if(strcmp(option,"NEW")==0){
		server.succ2_key=server.succ_key;
		server.succ2TCP=server.succTCP;
		strcpy(server.succ2IP, server.succIP);
		sscanf(buffer,"%s %d %s %d", option, &server.succ_key,server.succIP,&server.succTCP);
		close(succ_fd);
		succ_fd=createSocket(server.succIP,server.succTCP);
		sendSUCCONF(succ_fd);
		sendSUCC(pred_fd);
	}

	return;

}

int messageHandler(int afd, char *buffer){

	char option[10];


	if(!sscanf(buffer, " %s", option)) return 1;

	if(strcmp(option,"NEW")==0){
		{
			// if server has no successor
			if(server.node_key == server.succ_key && sscanf(buffer,"%s %d %s %d", option,&server.succ_key,server.succIP,&server.succTCP)==4){
				pred_fd=afd;
				succ_fd=createSocket(server.succIP, server.succTCP);
				maxfd=max(maxfd,succ_fd);
				sendSUCCONF(succ_fd);
				return 1;
			}
			else{
				sendSUCC(afd);
				sendBuffer(pred_fd, buffer);
				close(pred_fd);
				pred_fd=afd;
				maxfd=max(maxfd, afd);
				return 1;
			}
		}
			
	}
	else if(strcmp(option, "SUCCONF")==0){
		pred_fd=afd;
		return 1;
	}
	else if(strcmp(option, "FND")==0){

	}
	return 0;
}

void sendBuffer(int fd, char *buffer){

	ssize_t n;

	n=write(fd, buffer, strlen(buffer));
	if(n==-1)/*error*/exit(1);
}

void sendSUCC(int fd){
	char text[128];
	ssize_t n;
	
	sprintf(text,"SUCC %d %s %d \n", server.succ_key, server.succIP, server.succTCP);
	printf("text= %s",text);

	n=write (fd,text,strlen(text));
	if(n==-1)/*error*/exit(1);

}

void sendSUCCONF(int fd){
	char text[9]= "SUCCONF\n";
	ssize_t n;

	n=write (fd,text,strlen(text));
	if(n==-1)/*error*/exit(1);

}

int UserInput(){

	int i=-1, succi=0, succ_port=0 , k=0,port=0;
	char option[10]="\0", input[128]="\0", succIP[9]="\n",, ip[9]="\n";

	if(fgets(input, 128, stdin)==NULL){
		return 1;
	}

	if(!sscanf(input, " %s", option)) return 1;


	if(strcmp(option, "new")==0 && flag_new==0){ 

		if(sscanf(input, " %s %d", option, &i)==2 && i<MAX_KEY && i>0){
				CreateRing(i);
				return 0;
		}

		return 1;
	}

	else if(strcmp(option, "entry")==0){

		printf("%s selected\n", option);
		return 0;
	}

	else if(strcmp(option, "sentry")==0){

		if(sscanf(input, " %s %d %d %s %d", option, &i ,&succi, succIP, &succ_port)==5 && i<MAX_KEY && i>0){
				sentry(i, succi, succIP, succ_port);
				return 0;
		}
		return 1;
	}

	else if(strcmp(option, "leave")==0){

		printf("%s selected\n", option);
		return 0;
	}

	else if(strcmp(option, "show")==0){

		//printf("%s selected\n", option);
		ShowState();
		return 0;
	}

	else if(strcmp(option, "find")==0){

		printf("%s selected\n", option);
		if(sscanf(input, " %s %d %d %d %s %d", option, &k, &i, ip, &port)==5 && i<MAX_KEY){
				find(k,i,ip,port);
				return 0;
		}
		return 1;
	}

	else if(strcmp(option, "exit")==0){

		printf("%s selected\n", option);
		return -1;
	}


	return 1;
}

int CreateRing(int i){
	
	server.node_key = i;

	server.succ_key = i;
	server.succTCP= server.nodeTCP;
	strcpy(server.succIP, server.nodeIP);

	server.succ2_key = i;
	server.succ2TCP= server.nodeTCP;
	strcpy(server.succ2IP, server.nodeIP);

	flag_new=1;

	return 0;
}

void ShowState(){
	printf("\nState info:\n");
	printf("\t \t node IP and Port: %s :%d\n", server.nodeIP, server.nodeTCP);
	if(server.node_key == -1){
		return;
	}
	else{
		printf("\t \t node_key: %d\n\n", server.node_key);
		printf("\t \t succ IP and Port: %s :%d\n", server.succIP, server.succTCP);
		printf("\t \t succ_key: %d\n\n", server.succ_key);
		printf("\t \t succ2 IP and Port: %s :%d\n", server.succ2IP, server.succ2TCP);
		printf("\t \t succ2_key: %d\n", server.succ2_key);
	}
	return;
}

void storeInfo(int i, int succi, char *succIP, int succ_port){

	server.node_key = i;
	server.succ_key = succi;
	server.succ2_key = i;
	strcpy(server.succIP, succIP);
	strcpy(server.succ2IP, server.nodeIP);
	server.succTCP = succ_port;
	server.succ2TCP = server.nodeTCP;
	return;
}

void sentry(int i, int succi, char* succIP, int succ_port){

	int fd;
	char text[128];
	ssize_t n;

	flag_new=1;
	
	fd=createSocket(succIP, succ_port);
	storeInfo(i, succi, succIP, succ_port);

	//Prepares the "NEW" message
	sprintf(text,"NEW %d %s %d \n",i, server.nodeIP, server.nodeTCP);
	printf("text= %s",text);

	succ_fd=fd;
	maxfd=max(maxfd, succ_fd);

	//sends NEW
	n=write(succ_fd,text,strlen(text));
	if(n==-1)/*error*/exit(1);

	return;
}


//envia a mensagem NEW i i.IP i.port para o filedescripter fd (é usada em 2 contextos diferentes)
void neww(int fd,int i, char * ip, int port){
	char text[128];
	ssize_t n;
	
	//prepara o NEW
	sprintf(text,"NEW %d %s %d \n",i, ip, port);
	printf("text= %s\n",text);

	//envia o NEW
	n=write (fd,text,strlen(text));
	if(n==-1)/*error*/exit(1);
}

//envia a mensagem SUCC succ succ.IP succ.port para o filedescripter fd
void succ(int fd,int succ, char * succ_ip, int succ_port){
	char text[128];
	ssize_t n;
	
	//prepara o SUCC
	sprintf(text,"SUCC %d %s %d \n",succ, succ_ip, succ_port);
	printf("text= %s\n",text);

	//envia o NEW
	n=write (fd,text,strlen(text));
	if(n==-1)/*error*/exit(1);
}

int createSocket(char *ip, int p){

	struct addrinfo hints, *res;
	char port[9];
	int n;
	int fd=socket(AF_INET,SOCK_STREAM,0);//TCP socket
	if (fd==-1){printf("couldn't create socket\n"); exit(1);} //error

	memset(&hints, 0, sizeof(hints));
	hints.ai_family=AF_INET;
	hints.ai_socktype=SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	//transforma o succiPORTO numa string para usar no getadrinfo
	sprintf(port,"%d",p);

	errno=getaddrinfo(ip,port,&hints,&res) ;
	if(errno!=0) /*error*/ exit(1);

	n=connect (fd,res->ai_addr,res->ai_addrlen);
	if(n==-1)/*error*/exit(1);
	printf("connected\n");

	free(res);
	return fd;
}

void find(int k,int i,char *ip,int port){


}