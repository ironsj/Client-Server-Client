#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#include <pthread.h>

#define MAXLINE 4096 /*max text line length*/
#define SERV_PORT 3000 /*port*/

//extra 10 characters for "Client n: " (inform other clients who said what)
char recvline[MAXLINE + 10];

//each client will be able to constantly receive messages
void *receiveMessage(void *socket){
	int clientSocket = *((int *)socket);
	int n;
	while((n = recv(clientSocket, recvline, MAXLINE, 0)) > 0){
		fputs(recvline, stdout);
	}
	
}

int main(int argc, char **argv) {
	int sockfd;
	struct sockaddr_in servaddr;
	char sendline[MAXLINE];
	
	//each client has thread to receive messages from server
	pthread_t receive;

	//basic check of the arguments
	//additional checks can be inserted
	if (argc !=2) {
		perror("Usage: TCPClient <IP address of the server"); 
		exit(1);
	}

	//Create a socket for the client
	//If sockfd<0 there was an error in the creation of the socket
	if ((sockfd = socket (AF_INET, SOCK_STREAM, 0)) <0) {
		perror("Problem in creating the socket");
		exit(2);
	}

	//Creation of the socket
	memset(&servaddr, 0, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr= inet_addr(argv[1]);
	servaddr.sin_port =  htons(SERV_PORT); //convert to big-endian order

	//Connection of the client to the socket 
	if (connect(sockfd, (struct sockaddr *) &servaddr, sizeof(servaddr))<0) {
		perror("Problem in connecting to the server");
		exit(3);
	}
	
	//creates thread that is always ready for incoming messages
	pthread_create(&receive, NULL, (void*)receiveMessage, &sockfd);

	//always ready to send message to server which will then send to other clients
	while (fgets(sendline, MAXLINE, stdin) != NULL) {
		send(sockfd, sendline, strlen(sendline), 0);
	}
	
	//close thread
	pthread_join(receive, NULL);
	
	//close socket (connection to server)
	close(sockfd);
	

	exit(0);
}
