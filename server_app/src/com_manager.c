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

void *client_thread(void *sockfd) {
	
	int n;
	char buffer[BUFFER_SIZE];

	while(strcmp(buffer,"exit\n") != 0){
		listen(*(int *) sockfd, 10);
			
		/* read from the socket */
		bzero(buffer, BUFFER_SIZE);
		n = read(*(int *) sockfd, buffer, BUFFER_SIZE);
		if (n < 0) 
			printf("ERROR reading from socket");
		printf("Here is the message: %s", buffer);

		// Criando pacote para enviar
		packet package;
		package.type = DATA;
		package.seqn = 0;
		package.timestamp = time(NULL);
		//printf("Time: %ld\n", package.timestamp);
		package._payload = "Message received!";
		package.length = 18;
		// ..

		// Enviar mensagem
		send_packet(*(int *) sockfd, &package);
		// ..
	}

	printf("Encerrando conexao...\n");
	close(*(int *) sockfd);

	pthread_exit(NULL);
}

int send_packet(int socket, packet *package) {
	
	int n;

	// Enviando metadados
	n = write(socket, package, sizeof(packet));
	if (n < 0){
		printf("ERROR writing metadata to socket\n");
		return -1;
	}
	// ..

	// Enviando payload
	n = write(socket, package->_payload, package->length);
	if (n < 0){
		printf("ERROR writing data to socket\n");
		return -1;
	}
	// ..

	return 0;
	
}