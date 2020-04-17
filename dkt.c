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
#define MAX_KEY 100


extern int errno;
stateInfo server;
int flag; // flag = 0 if new has not been called and server is not connected
int pred_fd, succ_fd, udp_fd,u_fd; //udp_fd para quando é client, u_fd para quando é server
int maxfd, ekey;
socklen_t addrlen_udp;
struct sockaddr_in addr_udp;


int main(int argc, char *argv[]){

	int fd, newfd, afd=0, counter,i=0;
	ssize_t n,nread;
	socklen_t addrlen; //, addrlen_udp;
	struct addrinfo hints, *res;
	struct sockaddr_in addr;//, addr_udp;
	char buffer[128];
	fd_set rfds;
	enum {idle, busy} state;

	flag=0;
	pred_fd=-1;	// fd is not in use 
	succ_fd=-1;
	udp_fd=-1;
	u_fd=-1;
	ekey=0; // flag que controla quando se recebe um KEY se é para a entrada de um servidor ou se é só para o find

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

	u_fd=createSocketUDP_server(server.nodeTCP); //para poder ficar à escuta de sessões UDP

	state = idle;
	maxfd=fd;
	maxfd=max(maxfd,u_fd);

	while(1){
		FD_ZERO(&rfds);
		FD_SET(0,&rfds);	
		FD_SET(fd,&rfds);	
		if(pred_fd>0) FD_SET(pred_fd,&rfds);
		if(succ_fd>0) FD_SET(succ_fd,&rfds);
		if(u_fd>0) FD_SET(u_fd,&rfds);
	
		if(state==busy)	{
			FD_SET(afd,&rfds);
			maxfd=max(maxfd,afd);
		}
		counter=select(maxfd+1, &rfds, (fd_set*)NULL, (fd_set*)NULL, (struct timeval *)NULL);
		if(counter<=0) exit(1);

		if(flag == 1 && FD_ISSET(fd,&rfds)){ //aceitar ligações
			addrlen=sizeof(addr);
			if((newfd=accept(fd, (struct sockaddr*)&addr, &addrlen))==-1) exit(1);
			switch(state){
				case idle:	afd=newfd;
							state=busy;
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
			}
		}
		else if(FD_ISSET(0,&rfds)){
			i=UserInput();
			if(i==1){
				printf("Invalid command\n");
			}
			else if(i==-1){ 
				leave(); //closes sockets
				printf("Closing dkt\n"); 
				exit(1); //and exits
			}
		}
		else if(succ_fd>0 && FD_ISSET(succ_fd,&rfds)){

			if((n=read(succ_fd,buffer,128))!=0){
				if(n==-1) exit(1);
				write(1, "received from succ: ",20);
				write(1, buffer, n);
				succMessageHandler(buffer);
				maxfd=max(maxfd, succ_fd);
			}
			else{
				succLeft();
			}
		}
		else if(pred_fd>0 && FD_ISSET(pred_fd,&rfds)){ 

			if((n=read(pred_fd,buffer,128))!=0){
				if(n==-1) exit(1);
				write(1, "received from pred: ",20);
				write(1, buffer, n);
				predMessageHandler(buffer);
				maxfd=max(maxfd, pred_fd);
			}
		}
		else if(u_fd>0 && FD_ISSET(u_fd,&rfds)){

			addrlen_udp=sizeof(addr_udp); 
			nread= recvfrom (u_fd,buffer,128,0,(struct sockaddr*)&addr_udp,&addrlen_udp);
			if(nread==-1)/*error*/exit(1);
			write(1,"received from udp: ",19);
			write(1,buffer,nread);
			
			UDPMessageHandler(buffer);

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

void succLeft(){
	close(succ_fd);
	succ_fd=createSocket(server.succ2IP, server.succ2TCP);
	maxfd=max(maxfd, succ_fd);
	server.succ_key=server.succ2_key;
	strcpy(server.succIP, server.succ2IP);
	server.succTCP=server.succ2TCP;
	sendSUCCCONF(succ_fd);
	sendSUCC(pred_fd);  
}

void UDPMessageHandler(char *buffer){
	int i=0;
	char option[10];

	if(!sscanf(buffer, " %s", option)) return ;

	if(strcmp(option, "EFND")==0){
		if(sscanf(buffer, "%s %d", option,&i)==2){
				ekey=1; //acciona a flag para que a procura seja feita com o intuito de entrar um novo servidor
				find(i,server.node_key,server.nodeIP,server.nodeTCP);
		}
	}
}

void predMessageHandler(char *buffer){
	int k=0,i=0,port=0;
	char option[10],ip[9]="\n", text[128]="\n";
	ssize_t n;

	if(!sscanf(buffer, " %s", option)) return ;

	if(strcmp(option, "FND")==0){
		if(sscanf(buffer, "%s %d %d %s %d", option, &k, &i, ip, &port)==5){
				find(k,i,ip,port);
		}

	}
	else if(strcmp(option, "KEY")==0){ //no caso de a chave ser do original,recebe-se a mensagem KEY do predecessor em vez de se criar uma nova sessão TCP
		if(sscanf(buffer, "%s %d %d %s %d", option, &k, &i, ip, &port)==5){
				printf("Chave %d encontrada: %d %s %d\n",k,i,ip,port); //usei estes nomes para não ter de criar novas variáveis 

				if(ekey==1){// ou seja, se a procura foi feita para a entrada de um novo servidor, então vou enviar a info para ele

					sprintf(text,"EKEY %d %d %s %d\n",k,i,ip, port);

					n=sendto(u_fd,text,strlen(text),0,(struct sockaddr*)&addr_udp,addrlen_udp);
					if(n==-1) exit(1);

					ekey=0; //reset da flag, já foi feita a pesquisa para a entrada do novo servidor

				}
				return ;
		}
	}
}

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
		maxfd=max(maxfd,succ_fd);
		sendSUCCCONF(succ_fd);
		sendSUCC(pred_fd);
	}
	/*else if(strcmp(option, "EXIT")==0){
		printf("entrei EXIT\n");
		ShowState();
		if(server.nodeTCP != server.succTCP){ // enquanto há mais do que 1 servidor no anel
			sendBuffer(pred_fd,"EXIT\n");
		}
		leave();
		//printf("Closing dkt\n"); 
		//exit(1);
	}*/

	return;
}

int messageHandler(int afd, char *buffer){
	
	int k=0,i=0,port=0;
	char option[10],ip[9]="\n", text[128]="\n";
	ssize_t n;


	if(!sscanf(buffer, " %s", option)) return 1;

	if(strcmp(option,"NEW")==0){
		{
			// if server has no successor
			if(server.node_key == server.succ_key && sscanf(buffer,"%s %d %s %d", option,&server.succ_key,server.succIP,&server.succTCP)==4){
				pred_fd=afd;
				succ_fd=createSocket(server.succIP, server.succTCP);
				maxfd=max(maxfd,succ_fd);
				sendSUCCCONF(succ_fd);
				return 1;
			}
			else{
				sendSUCC(afd);
				sendBuffer(pred_fd, buffer);
				close(pred_fd);
				pred_fd=afd;

				return 1;
			}
		}
	}
	else if(strcmp(option, "SUCCCONF")==0){
		pred_fd=afd;
		sendSUCC(pred_fd);
		return 1;
	}
	else if(strcmp(option, "KEY")==0){
		if(sscanf(buffer, "%s %d %d %s %d", option, &k, &i, ip, &port)==5){
				printf("Chave %d encontrada: %d %s %d\n",k,i,ip,port); //usei estes nomes para não ter de criar novas variáveis 

				if(ekey==1){// ou seja, se a procura foi feita para a entrada de um novo servidor, então vou enviar a info para ele

					sprintf(text,"EKEY %d %d %s %d\n",k,i,ip, port);

					n= sendto (u_fd,text,strlen(text),0,(struct sockaddr*)&addr_udp,addrlen_udp);
					if(n==-1) exit(1);

					ekey=0; //reset da flag, já foi feita a pesquisa para a entrada do novo servidor

				}
				return 1;
		}
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
	
	n=write (fd,text,strlen(text));
	if(n==-1)/*error*/exit(1);
}

void sendSUCCCONF(int fd){
	char text[9]= "SUCCCONF\n";
	ssize_t n;

	n=write (fd,text,strlen(text));
	if(n==-1)/*error*/exit(1);
}

int UserInput(){
 
	int i=-1, succi=0, succ_port=0,k=0, boot=0,boot_port=0; //para ficar mais legível criei as variáveis "boot", apesar de poder usar as que já existiam com nomes menos intuitivos
	char option[10]="\0", input[128]="\0", succIP[9]="\n",bootIP[9]="\n";

	if(fgets(input, 128, stdin)==NULL){
		return 1;
	}

	if(!sscanf(input, " %s", option)) return 1;


	if(strcmp(option, "new")==0){ 
		if(flag==0){
			if(sscanf(input, " %s %d", option, &i)==2 && i<MAX_KEY && i>0){
				CreateRing(i);
				return 0;
			}
			else return 1;
		}
		else{
			printf("new has already been called\n");
		}

		return 0;
	}

	else if(strcmp(option, "entry")==0){

		if(sscanf(input, " %s %d %d %s %d", option,&i,&boot,bootIP,&boot_port )==5 && i<MAX_KEY && boot<MAX_KEY){ 
				entry(i,boot,bootIP,boot_port);
				return 0;
		}
		return 1;
	}

	else if(strcmp(option, "sentry")==0){

		if(sscanf(input, " %s %d %d %s %d", option, &i ,&succi, succIP, &succ_port)==5 && i<MAX_KEY && i>0){
				sentry(i, succi, succIP, succ_port);
				return 0;
		}
		return 1;
	}

	else if(strcmp(option, "leave")==0){
		leave();
		return 0;
	}

	else if(strcmp(option, "show")==0){
		ShowState();
		return 0;
	}

	else if(strcmp(option, "find")==0){
		if(sscanf(input, " %s %d", option, &k)==2 && k<MAX_KEY){
				find(k,server.node_key,server.nodeIP,server.nodeTCP);
				return 0;
		}
		return 1;
	}

	else if(strcmp(option, "exit")==0){
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

	flag=1;

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

// closes sockets and mantains the app open
void leave(){

	if(flag == 0){
		printf("Server is not connected to a ring yet\n");
		printf("If you want to close use 'exit'\n");
		return;
	}
	if(server.nodeTCP == server.succTCP){
		printf("I am lonely, bye\n");
		exit(1);
	}
	else{
		close(pred_fd);
		close(succ_fd);
		printf("bye\n");
		//exit(1);
	}
}

void sentry(int i, int succi, char* succIP, int succ_port){

	int fd;
	char text[128];
	ssize_t n;

	flag=1;
	
	fd=createSocket(succIP, succ_port);
	storeInfo(i, succi, succIP, succ_port);

	//Prepares the "NEW" message
	sprintf(text,"NEW %d %s %d \n",i, server.nodeIP, server.nodeTCP);

	succ_fd=fd;
	maxfd=max(maxfd, succ_fd);

	//sends NEW
	n=write(succ_fd,text,strlen(text));
	if(n==-1)/*error*/exit(1);

	return;
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

	free(res);
	return fd;
}

int createSocketUDP_server(int p){

	int fd;
	ssize_t n;
	struct addrinfo hints,*res;
	char port[9];

	fd=socket(AF_INET,SOCK_DGRAM,0); //UDP socket
	if(fd==-1) /*error*/exit(1);
	
	memset(&hints,0,sizeof(hints));
	hints.ai_family=AF_INET; // IPv4
	hints.ai_socktype=SOCK_DGRAM; // UDP socket
	hints.ai_flags=AI_PASSIVE;

	sprintf(port,"%d",p);

	errno= getaddrinfo (NULL,port,&hints,&res);
	if(errno!=0) /*error*/ exit(1);

	n= bind (fd,res->ai_addr, res->ai_addrlen);
	if(n==-1) /*error*/ exit(1);
	
	free(res);
	return fd;
}

void find(int k,int i,char *ip,int port){
	char text[128]="\n";
	int fd=0;
	ssize_t n;

	if(dist(k,server.succ_key)> dist(k,server.node_key)){//não é ele que tem a chave
		sprintf(text,"FND %d %d %s %d\n",k,i,ip,port);
		
		//então envia para o seu sucessor o FND para procurar
		n=write(succ_fd,text,strlen(text));
		if(n==-1)/*error*/exit(1);
		return;
	}
	else{
		//prepara o buffer para enviar
		sprintf(text,"KEY %d %d %s %d\n",k,server.succ_key,server.succIP,server.succTCP);

		if(i==server.succ_key){// a chave é do original e nesse caso não é necessário criar uma sessão TCP, basta enviar para o sucessor

			n=write(succ_fd,text,strlen(text));
			if(n==-1)/*error*/exit(1);
		}
		else{
			fd=createSocket(ip, port);
			maxfd=max(maxfd,fd);
			
			n=write(fd,text,strlen(text));
			if(n==-1)/*error*/exit(1);
			close(fd);
		}
		return;
	}
}

void entry(int i, int boot, char* bootIP, int boot_port){

	ssize_t n;
	socklen_t addrlen;
	struct addrinfo hints,*res;
	struct sockaddr_in addr;
	char buffer[128], text[128],option[9]="\n",ip[9]="\n";
	int k=0,porto=0;

	//udp_fd=createSocketUDP_client(boot_port,res);
	char port[9],buffer1[128];

	udp_fd=socket(AF_INET,SOCK_DGRAM,0);//UDP socket
	if (udp_fd==-1){printf("couldn't create socket\n"); exit(1);} //error

	memset(&hints, 0, sizeof(hints));
	hints.ai_family=AF_INET;
	hints.ai_socktype=SOCK_DGRAM;

	if(gethostname(buffer1,128)==-1) fprintf(stderr,"error: %s\n", strerror(errno));

	//transforma o porto p numa string para usar no getadrinfo
	sprintf(port,"%d",boot_port);

	errno=getaddrinfo(bootIP,port,&hints,&res) ;
	if(errno!=0) /*error*/ exit(1);

	sprintf(text,"EFND %d\n",i);

	n=sendto(udp_fd,text,strlen(text),0,res->ai_addr,res->ai_addrlen);
	if(n==-1) /*error*/ exit(1);

	addrlen=sizeof(addr);
	n= recvfrom (udp_fd,buffer,128,0,(struct sockaddr*)&addr,&addrlen);
	if(n==-1) exit(1);

	write(1,"echo: ",6);
	write(1,buffer,n);

	if(sscanf(buffer, "%s %d %d %s %d", option, &i, &k, ip, &porto)==5){
			if(k==i){
				printf("servidor já existe no anel\n");
			}
			else{
				sentry(i,k,ip,porto);
			}
	}

	freeaddrinfo(res);
	close (udp_fd);
}