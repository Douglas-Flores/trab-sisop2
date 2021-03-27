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
#include <semaphore.h>
#include "../lib/com_manager.h"

#define PORT 4000

sem_t sem_buffer;

int main(int argc, char *argv[]) {
  
  int sockfd;

  // Validando dados de entrada
  if (argc < 4) {
		fprintf(stderr,"usage %s hostname\n", argv[0]);
		return 0;
  }
  // ..
	
  // Estabelecendo conexão
  sockfd = connect_to_server(argv[2], argv[3]);
  if (sockfd < 0) {
    printf("ERROR stablishing connection, ending proccess ...\n");
    return 0;
  }
  // ..

  // Estabelecendo canal de notificações
  int notifsockfd;
  notifsockfd = connect_to_server(argv[2], argv[3]);
  if (sockfd < 0) {
    printf("ERROR stablishing connection, ending proccess ...\n");
    return 0;
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
  client_thread_args *args_cmd = malloc(sizeof(client_thread_args));
  args_cmd->sockfd = sockfd;
  client_thread_args *args_not = malloc(sizeof(client_thread_args));
  args_not->sockfd = notifsockfd;
  // ..

  sem_init(&sem_buffer, 0, 0);

  // Executa threads de envio de comandos e recepcao de notificacoes ate que terminem
  pthread_t cmd_thread, notif_thread;
  pthread_create(&cmd_thread, NULL, cmd_routine, args_cmd);
  pthread_create(&notif_thread, NULL, notif_routine, args_not);
  pthread_join(cmd_thread, NULL);
  pthread_cancel(notif_thread);
  pthread_join(notif_thread, NULL);
  // ..

  // Fechando o socket
	close(sockfd);
  printf("Connection closed.\n");
  // ..
  
  return 0;
}

// Thread do cliente para enviar comandos ao servidor
void *cmd_routine(void *args) {
  
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

    // Checando se é um comando
    if (buffer[0] == '\n' || buffer[0] == '\0')
      continue;
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
    bzero(&received, sizeof(packet));
    // ..

  }

  return NULL;
}

// Thread do cliente para receber notificacoes e imprimir na tela
void *notif_routine(void *args) {

  client_thread_args *rargs = args;
	int sockfd = rargs->sockfd;
  char buffer[BUFFER_SIZE];

	while(true){
    setbuf(stdout, NULL);
    bzero(buffer, BUFFER_SIZE); // limpando o buffer para leitura

    // Lendo do socket
    packet received;
    read_packet(sockfd, &received, buffer);
    printf("\nNEW NOTIFICATION: %s\n> ", received._payload);
    bzero(&received, sizeof(packet));
    // ..

    // Enviando resposta
    packet package;
    package.type = DATA;
    package.seqn = 0;
    package.timestamp = time(NULL);
    package._payload = malloc(sizeof("received"));
    strcpy(package._payload, "received");
    package.length = strlen("received");
    send_packet(sockfd, &package);
    free(package._payload);
    // ..

  }

  return NULL;
}
