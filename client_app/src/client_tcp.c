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

#define PORT 4000

int main(int argc, char *argv[]) {
  
  int sockfd, n;
  char buffer[BUFFER_SIZE];

  // Validando dados de entrada
  if (argc < 4) {
		fprintf(stderr,"usage %s hostname\n", argv[0]);
		exit(0);
  }
  // ..
	
  // Estabelecendo conexão
  sockfd = connect_to_server(argv[2], argv[3]);
  if (sockfd < 0) {
    printf("ERROR stablishing connection, ending proccess ...\n");
    exit(0);
  }
  // ..

  // Loop de interação
  while (5>1)
  {
    // Iniciando leitura de comandos
    printf("> ");
    bzero(buffer, BUFFER_SIZE);
    fgets(buffer, BUFFER_SIZE, stdin);
    // ..
      
    // Escrevendo no socket
    n = write(sockfd, buffer, strlen(buffer));
    if (n < 0) 
      printf("ERROR writing to socket\n");

    bzero(buffer, BUFFER_SIZE);
    // ..

    // Lendo do socket
    packet received;
    read_packet(sockfd, &received, buffer);
    bzero(&received, sizeof(packet));
    // ..

  /*bzero(buffer, 256);
  n = read(sockfd, buffer, 256);
  if (n < 0) 
	  printf("ERROR reading from socket\n");

  printf("%s\n", buffer);
  // ..*/
  }

  // Fechando o socket
	close(sockfd);
  // ..
  
  return 0;
}
