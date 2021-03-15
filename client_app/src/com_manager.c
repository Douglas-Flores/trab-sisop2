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
    char payload[128];

    // Lendo bytestream
    n = read(socket, buffer, BUFFER_SIZE);
    if (n < 0) 
	    printf("ERROR reading metadata from socket\n");
    // ..

    // Montando pacote
    package->type = buffer[0] | buffer[1] << 8;
    package->seqn = buffer[2] | buffer[3] << 8;
    package->length = buffer[4] | buffer[5] << 8;
    package->timestamp = buffer [8] | buffer[9] | buffer[10] | buffer[11] | buffer [12] | buffer [13] | buffer[14] | buffer[15] >> 8;
    package->_payload = payload;
    for(int i = 0; i < sizeof(packet); i++) {

    }

    // Montando payload
    for(int i = 0; i < 18; i++) {
        payload[i] = buffer[sizeof(packet)+i];
    }

    printf("Tipo: %d\n", package->type);
    printf("Seqn: %d\n", package->seqn);
    printf("Length: %d\n", package->length);
    printf("Time: %ld\n", package->timestamp);


    return n;
}
/*
    uint16_t type;          //Tipo do pacote (p.ex. DATA | CMD)
    uint16_t seqn;          //Número de sequência
    uint16_t length;        //Comprimento do payload 
    uint16_t timestamp;     // Timestamp do dado
    const char* _payload; 
*/