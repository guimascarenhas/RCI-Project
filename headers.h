#ifndef HEADERS_H_INCLUDED
#define HEADERS_H_INCLUDED

typedef struct stateInfo{
	int node_key;
	char nodeIP[15];
	int nodeTCP;
	int succ_key;
	char succIP[15];
	int succTCP;
	int succ2_key;
	char succ2IP[15];
	int succ2TCP;
	int pred_fd;
	int succ_fd;
}stateInfo;

int UserInput(int fd,struct addrinfo *res);
int ReceivedMessageDealer(int fd,char *buffer,int flag_new);
int CreateRing(int i);
void ShowState();
void sEntry(int fd,  int i, int succi, char *succiIP, int succiPORTO, int flag_new);
void neww(int fd,int i, char * ip, int port);
void succ(int fd,int i, char * ip, int port);

#endif // HEADERS_H_INCLUDED