#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>

#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#include <signal.h>
#include <sys/time.h>
#include <unistd.h>

#define PORT "58001"
#define max(A,B) ((A)>=(B)?(A):(B))

int main(int argc, char *argv[]){

    int aux=0;
    fd_set rfds;
    enum {idle,busy} state;
    int maxfd,counter,PORTO=0;
    char ID[10];

    int fd, newfd,afd=0, errcode;
    ssize_t n;
    socklen_t addrlen;
    struct addrinfo hints,*res;
    struct sockaddr_in addr;
    char buffer[128];

    if(argc==4){
        aux=strcmp(argv[1], "dkt");
         if(aux != 0){
            printf("1111Para iniciar a aplicação deve introduzir dkt <ID> <PORT>\n");
        }

    }
    else{
       printf("Para iniciar a aplicação deve introduzir dkt <ID> <PORT>\n"); 
    }

    PORTO= atoi(argv[3]);
    strcpy(ID,argv[2]);
    printf("ID: %s     PORTO=%d\n", ID, PORTO);

    /*------------------------------fd=socket(...);bind(fd,...);listen(fd,...); server TCP----------------------------------------*/
    fd=socket(AF_INET,SOCK_STREAM,0);
    if (fd==-1) exit(1); //error

    memset(&hints,0,sizeof hints);
    hints.ai_family=AF_INET;
    hints.ai_socktype=SOCK_STREAM;
    hints.ai_flags= AI_PASSIVE;

    errcode=getaddrinfo(NULL,PORT,&hints,&res);
    if((errcode)!=0) exit(1);
    printf("got addrinfo\n");    

    n=bind(fd,res->ai_addr,res->ai_addrlen);
    if(n==-1) /*error*/ exit(1);
    printf("bind done\n");

    if(listen(fd,5)==-1) exit(1);
    printf("listening\n");


    /*----------------------------SELECT----------------------------------------------------------------------------------------*/
    state=idle;
    while(1){
        FD_ZERO(&rfds);
        FD_SET(fd,&rfds);
        maxfd=fd;
        if(state==busy){FD_SET(afd,&rfds);maxfd=max(maxfd,afd);}

        counter=select(maxfd+1,&rfds,(fd_set*)NULL,(fd_set*)NULL,(struct timeval *)NULL);
        if(counter<=0)/*error*/exit(1);

        if(FD_ISSET(fd,&rfds)){
            addrlen=sizeof(addr);
            if((newfd=accept(fd,(struct sockaddr*)&addr,&addrlen))==-1)/*error*/exit(1);
            switch(state){
                case idle: afd=newfd;state=busy;break;
                case busy: 
                /* ... */
                //write “busy\n” in newfd
                write(newfd,"busy, go away!\n",16);
                close(newfd); break;
            }
        }
        if(FD_ISSET(afd,&rfds)){
            if((n=read(afd,buffer,128))!=0){
                if(n==-1)/*error*/exit(1);
                /* ... */
                //write buffer in afd
                write(1,"received: ",10);
                write(1,buffer,n);

                n=write(afd,buffer,n);
                if(n==-1) exit(1);
                
            }
            else{close(afd);state=idle;}//connection closed by peer
        }
    }//chaveta do while(1)

     close(fd);
     exit(0);
}