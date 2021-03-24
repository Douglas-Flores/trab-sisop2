#include <pthread.h>
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
  
  int sockfd;

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

  // Cria estrutura apenas com o sockfd (lista de seguidores eh ignorada)
  client_thread_args *args = malloc(sizeof(args));
  args->sockfd = sockfd;

  // Executa threads de envio de comandos e recepcao de notificacoes ate que terminem
  pthread_t cmd_thread, notif_thread;
  pthread_create(&cmd_thread, NULL, send_thread, args);
  pthread_create(&notif_thread, NULL, receive_thread, args);
  pthread_join(cmd_thread, NULL);
  pthread_join(notif_thread, NULL);

  // Fechando o socket
	close(sockfd);
  printf("Connection closed.\n");
  // ..
  
  return 0;
}

// Thread do cliente para enviar comandos ao servidor
void *send_thread(void *args) {
  
  char buffer[BUFFER_SIZE];
	client_thread_args *rargs = args;
	int sockfd = rargs->sockfd;

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

  }

  return NULL;
}

// Thread do cliente para receber notificacoes e imprimir na tela
void *receive_thread(void *args) {
  
  char buffer[BUFFER_SIZE];
	client_thread_args *rargs = args;
	int sockfd = rargs->sockfd;

	while(strcmp(buffer,"Invalid command, try again...") != 0){

    bzero(buffer, BUFFER_SIZE); // limpando o buffer para leitura

    // Lendo do socket
    packet received;
    read_packet(sockfd, &received, buffer);
    printf("%s\n", received._payload);
    // free(received._payload);
    bzero(&received, sizeof(packet));
    // ..

  }

  return NULL;
}
