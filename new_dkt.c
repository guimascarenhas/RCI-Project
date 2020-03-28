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
#define AAA printf("aqui\n");
#define MAX_KEY 100


extern int errno;
stateInfo server, aux_succi;


int main(int argc, char *argv[]){

	int fd, newfd, afd=0,maxfd, counter,i=0,j=0,flag_new=0;
	ssize_t n;
	socklen_t addrlen;
	struct addrinfo hints, *res;
	struct sockaddr_in addr;
	char buffer[128], info[5][20];
	fd_set rfds;
	enum {idle, busy} state;


	if(argc < 3){
		printf("Missing arguments when calling dkt\n");
		exit(1);
	}

	//stores host info 
	sprintf(server.nodeIP,"%s", argv[1]);
	server.nodeTCP= atoi(argv[2]);
	printf("server.nodeIP=%s \t server.nodeTCP=%d\n",server.nodeIP, server.nodeTCP );
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

	while(1){
		FD_ZERO(&rfds);
		FD_SET(0,&rfds);	
		FD_SET(fd,&rfds);	
		//fazer FD_SET do sucessor apenas quando existir!
		//fazer FD_SET do predecessor apenas quando existir!
		maxfd=fd;
		if(state==busy)	{//substituir o busy por cada uma das ligações,so se pode dar set de alho 
			FD_SET(afd,&rfds);
			maxfd=max(maxfd,afd);
		}

		counter=select(maxfd+1, &rfds, (fd_set*)NULL, (fd_set*)NULL, (struct timeval *)NULL);
		if(counter<=0) exit(1);

		if(FD_ISSET(fd,&rfds)){ //aceitar ligações
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

				j=ReceivedMessageDealer(afd,buffer,flag_new);
				if(j==1){
					printf("Invalid msg\n");
				}
			}
			else{
				close(afd);
				state = idle;
				printf("Client disconnected\n");
			}
		}

		//FD_ISSET do UDP
		//FD_ISSET do TCP dp +redecessor
		//FD_ISSET de TCP do sucessor

		if(FD_ISSET(0,&rfds)){
			i=UserInput(afd,res); // NOTA: ACHO QUE NÃO É ESTE fd QUE QUEREMOS <--- CHECKAR
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

int UserInput(int fd, struct addrinfo *res){

	int i=-1, succi=0, succiPORTO=0,flag=0, flag_new=0;
	char option[10]="\0", input[128]="\0", succiIP[9]="\n";

	if(fgets(input, 128, stdin)==NULL){
		return 1;
	}

	if(!sscanf(input, " %s", option)) return 1;


	if(strcmp(option, "new")==0 && flag<1){ //esta flag vai ser incrementada quando se cria o anel para que não se possa criar outro anel por cima 

		if(sscanf(input, " %s %d", option, &i)==2 && i<MAX_KEY){
				//printf("%s selected\n", option);
				CreateRing(i);
				flag++;
				return 0;
		}

		return 1;
	}

	else if(strcmp(option, "entry")==0){

		printf("%s selected\n", option);
		return 0;
	}

	else if(strcmp(option, "sentry")==0){

		if(sscanf(input, " %s %d %d %s %d", option, &i ,&succi, succiIP, &succiPORTO)==5 && i<MAX_KEY){
				//printf("%s selected\n", option);
				sEntry(fd, i, succi, succiIP, succiPORTO,flag_new);
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
		return 0;
	}

	else if(strcmp(option, "exit")==0){

		printf("%s selected\n", option);
		return -1;
	}


	return 1;
}

int ReceivedMessageDealer(int fd,char *buffer,int flag_new){
	char info[5][20];

	printf("%s\n", buffer);

	if(!sscanf(buffer, " %s", info[0])) return 1;

	if(strcmp(info[0],"SUCCCONF")==0){
		if(sscanf(buffer,"%s",info[0])==1){

			return 0;
		}
		else{printf("BUG-SUCCCONF!\n");return 1;}

	}
	else if(strcmp(info[0],"EFND")==0){
		if(sscanf(buffer,"%s %s",info[0],info[1])==2){
			//info[1] é a chave que se quer que o servidor que recebeu esta msg procure, ou seja, este servidor tem de procurar por esta chave

			return 0;
		}
		else{printf("BUG-EFND!\n");return 1;}
		
	}
	else if(strcmp(info[0],"NEW")==0){
		if(flag_new==1){//caso 1 do uso do NEW - um servidor entrante informa o seu futuro sucessor que quer entrar com a chave i

			if(sscanf(buffer,"%s %d %s %d",info[0],&server.succ_key,server.succIP,&server.succTCP)==4){
			printf("NEW: info 0:%s \t info 1:%s \t info 2:%s \t info 3:%s \n",info[0],info[1],info[2],info[3] );}
			else{
				printf("Erro NEW 1\n");
				return 1;
			}
			
			//envia SUCC para fd
			succ(fd,server.succ2_key,server.succ2IP,server.succ2TCP);
			
			return 0;
		}
		else if(flag_new==2){//caso 2 do NEW - um servidor informa o seu atual predecessor que o servidor de info (i,i.IP,i.port) quer entrar no anel , para que o predecessor estableleça o servidor como seu sucessor

			return 0;
		}
		else{printf("BUG-NEW!\n");return 1;}
	}
	else if(strcmp(info[0],"SUCC")==0){
		if(sscanf(buffer,"%s %d %s %d",info[0],&server.succ2_key,server.succ2IP,&server.succTCP)==4){
			printf("SUCC: info 0:%s \t info 1:%s \t info 2:%s \t info 3:%s \n",info[0],info[1],info[2],info[3] );
			return 0;
		}
		else{printf("BUG-SUCC!\n");return 1;}
		
	}
	else if(strcmp(info[0],"FND")==0){
		if(sscanf(buffer,"%s %s %s %s %s",info[0],info[1],info[2],info[3],info[4])==5){

			return 0;
		}
		else{printf("BUG-FND!\n"); return 1;}
		
	}
	else if(strcmp(info[0],"KEY")==0){
		if(sscanf(buffer,"%s %s %s %s %s",info[0],info[1],info[2],info[3],info[4])==5){

			return 0;
		}
		else{printf("BUG-KEY!\n"); return 1;}
				
	}
	else if(strcmp(info[0],"EKEY")==0){
		if(sscanf(buffer,"%s %s %s %s %s",info[0],info[1],info[2],info[3],info[4])==5){

		return 0;
		}
		else{printf("BUG-EKEY!\n"); return 1;}
	}
	else{
		return 1;
	}

	
	return 0;
}

int CreateRing(int i){
	
	server.node_key = i;

	server.succ_key = i;
	server.succTCP= server.nodeTCP;
	strcpy(server.succIP, server.nodeIP);

	server.succ2_key = i;
	server.succ2TCP= server.nodeTCP;
	strcpy(server.succ2IP, server.nodeIP);

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

void sEntry(int fd, int i, int succi, char *succiIP, int succiPORTO, int flag_new){
	int n=0;
	char text[50], PORT[9];
	struct addrinfo hints, *res;

	//printf("sEntry: nodeIP= %s \t %d %d %s %d\n",server.nodeIP,i,succi, succiIP,succiPORTO );

	fd=socket(AF_INET,SOCK_STREAM,0);//TCP socket
	if (fd==-1){printf("couldn't create socket\n"); exit(1);} //error

	memset(&hints, 0, sizeof(hints));
	hints.ai_family=AF_INET;
	hints.ai_socktype=SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	//transforma o succiPORTO numa string para usar no getadrinfo
	sprintf(PORT,"%d",succiPORTO);

	errno=getaddrinfo(succiIP,PORT,&hints,&res) ;
	if(errno!=0) /*error*/ exit(1);

	//guarda as info sobre este servidor antes de fazer uma ligação no caso em que só há 1 servidor no anel
	server.node_key = i;
	server.succ_key = succi;
	server.succ2_key = i;
	strcpy(server.succIP, succiIP);
	strcpy(server.succ2IP, server.nodeIP);

	n= connect (fd,res->ai_addr,res->ai_addrlen);
	if(n==-1)/*error*/exit(1);
	printf("connected\n");

	//envia o new para fd
	neww(fd, i,server.nodeIP, server.nodeTCP);
	
	//recebe um SUCC
	if((n=read(fd,text,50))!=0){
		printf("SUCC recebido: %s\n",text );
		ReceivedMessageDealer(fd,text,flag_new);
	}

	//freeaddrinfo(res);
	//close (fd);
}

//envia a mensagem NEW i i.IP i.port para o o filedescripter fd (é usada em 2 contextos diferentes)
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

//envia a mensagem SUCC succ succ.IP succ.port para o o filedescripter fd
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