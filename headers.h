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

int dist(int k, int l);
int UserInput();
int CreateRing(int i);
void ShowState();
int createSocket(char *ip, int p);
int createSocketUDP_client(int p, struct addrinfo * res);
int createSocketUDP_server(int p);
int messageHandler(int afd, char* buffer);
void succMessageHandler(char *buffer);
void predMessageHandler(char *buffer);
void UDPMessageHandler(char *buffer);
void sentry(int i, int succi, char* succIP, int succ_port);
void leave();
void sendSUCCCONF(int fd);
void sendSUCC(int fd);
void sendBuffer(int fd, char *buffer);
void succLeft();
void find(int k,int i,char *ip,int port);
void entry(int i, int boot, char* bootIP, int boot_port);


#endif // HEADERS_H_INCLUDED