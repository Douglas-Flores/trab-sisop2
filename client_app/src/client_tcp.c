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

  // Autenticação
  if (authenticate(sockfd, argv[1]) < 0) {
    printf("Falha de Autenticacao. Tente novamente.\n");
    close(sockfd);
    return 0;
  }
  // ..

  pthread_t cmd_thread, notif_thread;
  pthread_create(&cmd_thread, NULL, send_thread, sockfd);
  // pthread_create(&notif_thread, NULL, receive_thread, NULL);

  pthread_join(cmd_thread);
  // pthread_join(notif_thread);

  // Fechando o socket
	close(sockfd);
  printf("Connection closed.\n");
  // ..
  
  return 0;
}

// Thread do cliente para enviar comandos ao servidor
void *send_thread(int sockfd) {
  
  char buffer[BUFFER_SIZE];

  // Loop de interação
  while (strcmp(buffer, "exit\n") != 0) {
    // Iniciando leitura de comandos
    printf("> ");
    bzero(buffer, BUFFER_SIZE);
    fgets(buffer, BUFFER_SIZE, stdin);
    // ..

    // Checando comando de exit
    if (strcmp(buffer, "exit\n") == 0)
      break;
    // ..
      
    // Escrevendo no socket
    if(strlen(buffer) > 1) {
      packet package;
      package.type = DATA;
      package.seqn = 0;
      package.timestamp = time(NULL);
      package._payload = buffer;
      package.length = strlen(buffer);
      send_packet(sockfd, &package);
    }

    bzero(buffer, BUFFER_SIZE); // limpando o buffer para leitura

    // Lendo do socket
    packet received;
    read_packet(sockfd, &received, buffer);
    printf("%s\n", received._payload);
    free(received._payload);
    bzero(&received, sizeof(packet));
    bzero(buffer, BUFFER_SIZE);
    // ..

  }

}

// Thread do cliente para receber notificacoes e imprimir na tela
void *receive_thread(void *args) {
  // TODO
}
