#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>

#define PORT 3300

int clientFd;
struct sockaddr_in cliaddr;

void startServer(){
	int listenfd;
	socklen_t clilen;
	struct sockaddr_in servaddr;

	clientFd = 0;

	listenfd=socket(AF_INET,SOCK_STREAM,0);

	memset(&servaddr, 0 , sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr=htonl(INADDR_ANY);
	servaddr.sin_port=htons(PORT);
	bind(listenfd,(struct sockaddr *)&servaddr,sizeof(servaddr));

	listen(listenfd,5);

	clientFd = accept(listenfd,(struct sockaddr *)&cliaddr,&clilen);
	close (listenfd);
}

void sendToServer(char *data, int len){
	if(clientFd>0){
		sendto(clientFd,data,len,0,(struct sockaddr *)&cliaddr,sizeof(cliaddr));
	}
}

void cleanupServer(){
	close(clientFd);
	clientFd = 0;
}

