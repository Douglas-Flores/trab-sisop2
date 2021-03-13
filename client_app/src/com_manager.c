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
    int sockfd, n;
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