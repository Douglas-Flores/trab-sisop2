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