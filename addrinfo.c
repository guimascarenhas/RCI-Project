#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#define NAME_LEN 128

void GetHostInfo();

int main(){

	char name[NAME_LEN], address[INET_ADDRSTRLEN];
	long unsigned int addrHexadecimal; 

	GetHostInfo(name, address, &addrHexadecimal);

	printf("canonical hostname: %s\n", name);
	printf("internet access: %s (%08lx)\n", address, addrHexadecimal);

	return 0;
}

void GetHostInfo(char name[NAME_LEN], char address[INET_ADDRSTRLEN], long unsigned int *addrHexadecimal){

	struct addrinfo hints, *res, *p;
	struct in_addr *addr;
	int errcode;
	char serverAddress[INET_ADDRSTRLEN];

	// gets name of host and writes it in name[NAME_LEN]
	if(gethostname(name,NAME_LEN)==-1){
		fprintf(stderr, "error %s\n", strerror(errno));
		exit(0);
	}

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET; // IPV4
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_CANONNAME;

	if((errcode=getaddrinfo(name, NULL, &hints, &res)) !=0){
		fprintf(stderr, "error: getaddrinfo: %s\n", gai_strerror(errcode));
		exit(0);
	}
	else{
		for(p=res; p!=NULL; p=p->ai_next){
			addr=&((struct sockaddr_in *)p->ai_addr)->sin_addr;
			inet_ntop(p->ai_family, addr, serverAddress, sizeof(serverAddress));
		}
		strcpy(address, serverAddress);
		*addrHexadecimal= (long unsigned int)ntohl(addr->s_addr);
		freeaddrinfo(res);
		}
	return;
}
