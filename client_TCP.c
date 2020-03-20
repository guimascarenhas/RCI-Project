#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#define PORT "58001"

int main(){
	int fd,errcode;
	ssize_t n;
	socklen_t addrlen;
	struct addrinfo hints,*res;
	struct sockaddr_in addr;
	char buffer[128],buffer1[128], text[128];

	fd=socket(AF_INET,SOCK_STREAM,0);//TCP socket
	if (fd==-1) exit(1); //error

	memset(&hints,0,sizeof hints);
	hints.ai_family=AF_INET;  //IPv4
	hints.ai_socktype=SOCK_STREAM;//TCP socket	

	if(gethostname(buffer1,128)==-1) fprintf(stderr,"error: %s\n", strerror(errno));
	else printf("hostname:%s ", buffer1);

	errcode=getaddrinfo(buffer1,PORT,&hints,&res) ;
	if(errcode!=0) /*error*/ exit(1);
	
	n= connect (fd,res->ai_addr,res->ai_addrlen);
	if(n==-1)/*error*/exit(1);
	printf("connected\n");

	printf("ENTER TEXT:\n");
	scanf("%s",text);
	strcat(text,"\n");
	n=write (fd,text,strlen(text));
	if(n==-1)/*error*/exit(1);

	n= read (fd,buffer,128);
	if(n==-1)/*error*/exit(1);

	write(1,"echo: ",6);
	write(1,buffer,n);

	freeaddrinfo(res);
	close (fd);
	return 0;
}