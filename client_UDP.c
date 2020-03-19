#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
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
	char buffer[128], buffer1[128];
	extern int errno;

	fd= socket(AF_INET,SOCK_DGRAM,0); //UDP socket
	if(fd ==-1) /*error*/
		exit(1);

	memset(&hints,0,sizeof (hints));
	hints.ai_family=AF_INET; //IPv4
	hints.ai_socktype=SOCK_DGRAM; //UDP socket

	if(gethostname(buffer1,128)==-1) fprintf(stderr,"error: %s\n", strerror(errno));
	else printf("hostname:%s ", buffer1);

	errcode=getaddrinfo(buffer1,PORT,&hints,&res) ;
	if(errcode!=0) /*error*/ exit(1);

	n=sendto(fd,"Hello!\n",7,0,res->ai_addr,res->ai_addrlen);
	if(n==-1) /*error*/ exit(1);

	addrlen=sizeof(addr);
	n= recvfrom (fd,buffer,128,0,(struct sockaddr*)&addr,&addrlen);
	if(n==-1) /*error*/ exit(1);

	write(1,"echo: ",6);
	write(1,buffer,n);

	freeaddrinfo(res);
	close (fd);
	return 0;
}