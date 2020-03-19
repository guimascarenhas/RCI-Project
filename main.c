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

int main(int argc, char *argv[]){

    int aux=0;

    if(argc==4){
        aux=strcmp(argv[1], "dkt");
         if(aux != 0){
            printf("1111Para iniciar a aplicação deve introduzir dkt <ID> <PORT>\n");
        }

    }
    else{
       printf("Para iniciar a aplicação deve introduzir dkt <ID> <PORT>\n"); 
       exit(0);
    }

    return 0;
}