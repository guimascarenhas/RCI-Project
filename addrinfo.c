#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>


int main(){

	struct addrinfo hints, *res, *p;
	int errcode;
	char buffer[INET_ADDRSTRLEN];
	char bufferino[128];
	struct in_addr *addr;


	if(gethostname(bufferino,128)==-1){
		fprintf(stderr, "error %s\n", strerror(errno));
		exit 0;
	}


	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET; // IPV4
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_CANONNAME;

	if((errcode=getaddrinfo(bufferino, NULL, &hints, &res)) !=0)
		fprintf(stderr, "error: getaddrinfo: %s\n", gai_strerror(errcode));
	else{
		printf("canonical hostname: %s\n", res->ai_canonname);
		for(p=res; p!=NULL; p=p->ai_next){
			addr=&((struct sockaddr_in *)p->ai_addr)->sin_addr;
			printf("internet access: %s (%08lx)\n", inet_ntop(p->ai_family, addr, buffer, sizeof(buffer)), (long unsigned int)ntohl(addr->s_addr));
			} 
		freeaddrinfo(res);
		}
	exit(0);
}
