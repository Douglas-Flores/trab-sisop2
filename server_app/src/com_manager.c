#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <time.h>
#include <pthread.h>
#include <errno.h>
#include "../lib/com_manager.h"

#define PORT 4000

int open_socket()
{
	int sockfd, newsockfd, n;
	socklen_t clilen;
	char buffer[256];
	struct sockaddr_in serv_addr, cli_addr;
	
	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) 
        printf("ERROR opening socket");
	
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(PORT);
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	bzero(&(serv_addr.sin_zero), 8);     
    
    // Binding
	if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) 
		printf("ERROR on binding");
    // ..
	
	while (5>1)
	{
	
		listen(sockfd, 5);
		
		clilen = sizeof(struct sockaddr_in);
		if ((newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen)) == -1) 
			printf("ERROR on accept");
		
		bzero(buffer, 256);
		
		/* read from the socket */
		n = read(newsockfd, buffer, 256);
		if (n < 0) 
			printf("ERROR reading from socket");
		printf("Here is the message: %s\n", buffer);
		
		/* create packet to send*/
		packet package;
		package.type = DATA;
		package.seqn = 0;
		package.timestamp = time(NULL);
		package._payload = "I hear you babe!";
		package.length = 16;

		/* write in the socket */
		n = write(newsockfd, package._payload, strlen(package._payload));
		if (n < 0) 
			printf("ERROR writing to socket");
	}

	close(newsockfd);
	close(sockfd);
	return 0; 
}

void *client_thread(void *sockfd)
{
	int n;
	char buffer[256];
	
	bzero(buffer, 256);
		
	/* read from the socket */
	n = read(*(int *) sockfd, buffer, 256);
	if (n < 0) 
		printf("ERROR reading from socket");
	printf("Here is the message: %s\n", buffer);

	/* create packet to send*/
	packet package;
	package.type = DATA;
	package.seqn = 0;
	package.timestamp = time(NULL);
	package._payload = "I hear you babe!";
	package.length = 16;

	/* write in the socket */
	n = write(*(int *) sockfd, package._payload, strlen(package._payload));
	if (n < 0) 
		printf("ERROR writing to socket");

	close(*(int *) sockfd);

	pthread_exit(NULL);
}

int send_packet(int socket, packet package)
{
	int n = write(socket, &package, sizeof(package));
	if (n < 0){
		printf("ERROR writing to socket");
		return -1;
	}

	return 0;
	
}