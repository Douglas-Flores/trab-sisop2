#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include "../lib/com_manager.h"

int connect_to_server(char *end, char *port){
    int sockfd;
    struct sockaddr_in serv_addr;
    struct hostent *server;
    struct in_addr addr_sent;
        
    // Procurando servidor
    addr_sent.s_addr = inet_addr(end);  // obtendo o endereço IP fornecido pelo usuário
    server = gethostbyaddr(&addr_sent, sizeof(struct in_addr), AF_INET);  // buscando o servidor pelo endereço IP
    if (server == NULL) {   // saindo caso o servidor não tenha sido encontrado
        fprintf(stderr,"ERROR, no such host\n");
        exit(0);
    }
    // ..
    
    // Abrindo o socket
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        printf("ERROR opening socket\n");
        return -1;
    }
    // ..

    // Fornecendo informações sobre o servidor
    serv_addr.sin_family = AF_INET;   // usa IPv4     
    serv_addr.sin_port = htons(atoi(port));  // definindo porta de conexão 
    serv_addr.sin_addr = *((struct in_addr *)server->h_addr); // definindo endereço IP
    bzero(&(serv_addr.sin_zero), 8);
    // ..

    // Estabelecendo conexão
    if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) { 
        printf("ERROR connecting\n");
        return -1;
    }
    // ..

    return sockfd;
}

int read_packet(int socket, packet *package, char *buffer) {
    int n;
    char *payload = malloc(sizeof(char)*128);

    // Lendo bytestream
    n = read(socket, buffer, BUFFER_SIZE);
    if (n < 0) {
	    printf("ERROR reading metadata from socket\n");
        return n;
    }
    // ..

    // Montando pacote
    package->type = buffer[0] | buffer[1] << 8;
    package->seqn = buffer[2] | buffer[3] << 8;
    package->length = buffer[4] | buffer[5] << 8;
    //package->timestamp = buffer [8] | buffer[9] | buffer[10] | buffer[11] | buffer [12] | buffer [13] | buffer[14] | buffer[15];
    package->_payload = payload;
    
    if (package->length > BUFFER_SIZE)
        return -1;

    // Montando payload
    bzero(payload,128);
    for(int i = 0; i < package->length; i++) {
        payload[i] = buffer[sizeof(packet)+i];
    }

    return n;
}

int send_packet(int socket, packet *package) {
	
	int n;

	// Enviando metadados
	/*n = write(socket, package, sizeof(packet));
	if (n < 0){
		printf("ERROR writing metadata to socket\n");
		return -1;
	}*/
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

int authenticate(int socket, char *username) {
    char buffer[BUFFER_SIZE];
    int n=0;

    // Enviando nome de usuário
    packet package;
    package.type = DATA;
    package.seqn = 0;
    package.timestamp = time(NULL);
    package._payload = username;
    package.length = strlen(username);
    send_packet(socket, &package);
    // ..

    // Recebendo resposta
    packet *rcv = (packet *) malloc(sizeof(packet));
    read_packet(socket, rcv, buffer);
    // ..

    // Verificando resposta
    if(strcmp(rcv->_payload, "success") == 0)
        n = 0;
    else
        n = -1;
    // ..
    free(rcv->_payload);
    free(rcv);

    return n;
}