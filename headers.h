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
}stateInfo;

int UserInput();
int ReceivedMessageDealer(int fd,char *buffer,int flag_new);
int CreateRing(int i);
void ShowState();
//void sEntry(int fd,  int i, int succi, char *succiIP, int succiPORTO, int flag_new);
void neww(int fd,int i, char * ip, int port);
void succ(int fd,int i, char * ip, int port);
int createSocket(char *, int p);
int messageHandler(int afd, char* buffer);
void succMessageHandler(char *buffer);
void sentry(int i, int succi, char* succIP, int succ_port);
void sendSUCCONF(int fd);
void sendSUCC(int fd);
void sendBuffer(int fd, char *buffer);

#endif // HEADERS_H_INCLUDED