#ifndef HEADERS_H_INCLUDED
#define HEADERS_H_INCLUDED

typedef struct stateInfo{
	int node_key;
	char nodeIP[20];
	int succ_key;
	char succIP[20];
	int succ2_key;
	char succ2IP[20];
}stateInfo;

int UserInput(int fd,struct addrinfo *res);
int ReceivedMessageDealer(int fd,char *buffer);
int CreateRing(int i);
void ShowState();
void sEntry(int fd,  int i, int succi, char *succiIP, int succiPORTO);

#endif // HEADERS_H_INCLUDED