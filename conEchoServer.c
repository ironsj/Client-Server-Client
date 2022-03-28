#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

#define MAXLINE 4096 /*max text line length*/
#define SERV_PORT 3000 /*port*/
#define LISTENQ 8 /*maximum number of client connections*/

int numClients = 0;
pthread_mutex_t mutex;
int clients[LISTENQ];

void sendToOthers(char *buf, int currSocket){
	pthread_mutex_lock(&mutex);
	for(int i = 0; i < numClients; i++){
		if(clients[i] != currSocket){
			send(clients[i], buf, MAXLINE, 0);
		}
	}
	pthread_mutex_unlock(&mutex);
}

void *receiveMessage(void *client_socket){
	int socket = *((int *)client_socket);
	char buf[MAXLINE];
	int n;
	while((n = recv(socket, buf, MAXLINE, 0) > 0)){
		char message[MAXLINE + 10];
		for(int i = 0; i < numClients; i++){
			if(clients[i] == socket){
				printf("Received from client %d: ", i);
				puts(buf);
				sprintf(message, "Client %d: %s", i, buf);
			}
		}
		sendToOthers(message, socket);
		memset(&buf, '\0', MAXLINE);
	}
}

int main (int argc, char **argv)
{
	int listenfd, connfd, n;
	pthread_t receive;
	socklen_t clilen;
	char buf[MAXLINE];
	struct sockaddr_in cliaddr, servaddr;

	//Create a socket for the soclet
	//If sockfd<0 there was an error in the creation of the socket
	if ((listenfd = socket (AF_INET, SOCK_STREAM, 0)) <0) {
		perror("Problem in creating the socket");
		exit(2);
	}


	//preparation of the socket address
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port = htons(SERV_PORT);

	//bind the socket
	bind (listenfd, (struct sockaddr *) &servaddr, sizeof(servaddr));

	//listen to the socket by creating a connection queue, then wait for clients
	listen (listenfd, LISTENQ);

	printf("%s\n","Server running...waiting for connections.");

	for ( ; ; ) {

		clilen = sizeof(cliaddr);
		//accept a connection
		connfd = accept (listenfd, (struct sockaddr *) &cliaddr, &clilen);
		pthread_mutex_lock(&mutex);
		clients[numClients] = connfd;
		numClients++;
		printf("%s\n","Received request...");
		pthread_create(&receive, NULL, (void *)receiveMessage, &connfd);
		pthread_mutex_unlock(&mutex);

	}

}
