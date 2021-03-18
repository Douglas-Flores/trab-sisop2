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
#include "../lib/database.h"

#define PORT 4000

profile_list *profiles;

void *client_thread(void *args) {
	
	int n, sockfd, userid;
	char buffer[BUFFER_SIZE];
	profile *cur_user;
	client_args *rargs = args;
	setbuf(stdout, NULL);

	sockfd = rargs->sockfd;
	profiles = rargs->profiles;

	userid = authenticate(sockfd);
	if (userid < 0) {
		printf("Falha de Autenticacao.\n");
		close(sockfd);
		return 0;
	}

	profile_list *node = profiles;
	for (int i = 0; i < userid; i++)
		node = node->next;
	cur_user = node->profile;

	while(strcmp(buffer,"exit\n") != 0){

		// Lendo do Socket
		bzero(buffer, BUFFER_SIZE);
		n = read(sockfd, buffer, BUFFER_SIZE);
		if (n < 0) {
			printf("ERROR reading from socket");
			continue;
		}
		else if (buffer[0] == '\0'){
			printf("No response from user.\n");
			break;
		}
		else
			printf("%s: %s", cur_user->username, buffer);
		// ..

		// Interpretando comando
		char *cmd = strtok(buffer," ");
		char *data = strtok(NULL, "\n");
		char response[256] = " ";
		if(strcmp(cmd,"FOLLOW") == 0)
			follow(profiles, cur_user, data, response);
		// ..

		// Criando pacote para enviar
		packet package;
		package.type = DATA;
		package.seqn = 0;
		package.timestamp = time(NULL);
		package._payload = response;
		package.length = strlen(response);
		// ..

		// Enviar mensagem
		if (send_packet(sockfd, &package) < 0)
			break;
		// ..

		bzero(response, 256);
	}

	printf("%s disconnected...\n", cur_user->username);
	close(sockfd);

	return 0;
}

int send_packet(int socket, packet *package) {
	
	int n;

	// Enviando metadados
	n = write(socket, package, sizeof(packet));
	if (n < 0){
		printf("ERROR writing metadata to socket\n");
		n = -1;
	}
	// ..

	// Enviando payload
	n = write(socket, package->_payload, package->length);
	if (n < 0){
		printf("ERROR writing data to socket\n");
		n = -1;
	}
	// ..

	return n;
	
}

int read_packet(int socket, packet *package, char *buffer) {
    int n;
    char payload[128]="";

    // Lendo bytestream
    n = read(socket, buffer, BUFFER_SIZE);
    if (n < 0) {
	    printf("ERROR reading from socket\n");
		n = -1;
	}
    // ..

    // Montando pacote
    package->type = buffer[0] | buffer[1] << 8;
    package->seqn = buffer[2] | buffer[3] << 8;
    package->length = buffer[4] | buffer[5] << 8;
    //package->timestamp = buffer [8] | buffer[9] | buffer[10] | buffer[11] | buffer [12] | buffer [13] | buffer[14] | buffer[15];
    package->_payload = payload;

    // Montando payload
    for(int i = 0; i < 7; i++) {
        payload[i] = buffer[i];
    }
	printf("Here is the message: %s\n", payload);

    return n;
}

int authenticate(int socket){
	int n;
	char buffer[BUFFER_SIZE];

	// Lendo username
	bzero(buffer, BUFFER_SIZE);
	n = read(socket, buffer, BUFFER_SIZE);
	if (n < 0) 
		printf("ERROR reading from socket");
	// ..

	// Verificando existência do usuário
	n = get_profile(buffer, profiles);
	// ..

	// Criando pacote para enviar
	packet package;
	package.type = DATA;
	package.seqn = 0;
	package.timestamp = time(NULL);
	if (n >= 0) {
		package._payload = "success";
		printf("%s logged\n", buffer);
	}
	else {
		package._payload = "failure";
		printf("%s attempted to log but was unsuccessful.\n", buffer);
	}
	package.length = strlen(package._payload) + 1;
	// ..

	// Enviando resposta
	send_packet(socket, &package);
	// ..

	return n;
}