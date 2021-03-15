#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <errno.h>
#include <ctype.h>
#include <time.h>
#include <arpa/inet.h>
#include "../lib/com_manager.h"

#define PORT 4000

// SOMENTE PARA TESTES ------------------------------------------
#define handle_error_en(en, msg) \
  do { errno = en; perror(msg); exit(EXIT_FAILURE); } while (0)

#define handle_error(msg) \
  do { perror(msg); exit(EXIT_FAILURE); } while (0)
// -------------------------------------------------------------

int main(int argc, char *argv[])
{
  int sockfd, option = 1;
	struct sockaddr_in serv_addr;
	
  // Abrindo socket
	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    printf("ERROR opening socket\n");
  setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option));  // necessário para reutilizar o socket assim que ele for fechado 
	// ..

  // Definindo endereço do servidor
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(PORT);
  serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
	bzero(&(serv_addr.sin_zero), 8);
  // ..   
    
  // Binding
	if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) 
		printf("ERROR on binding\n");
  // ..

  // Loop de leitura por novas requisições de conexão
  while (2>1){  // TODO: condição de saida
    if (listen(sockfd, BACKLOG_MAX) == 0)
    {
      int newsockfd;
      socklen_t clilen;
      struct sockaddr_in cli_addr;

      clilen = sizeof(struct sockaddr_in);
      if ((newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen)) == -1)
        printf("ERROR on accept\n");

      // Criando thread para o novo usuário conectado  
      pthread_t th;
      pthread_create(&th, NULL, client_thread, &newsockfd);
      // ..
    }
  }
  // ..

	if (close(sockfd) < 0)
    printf("ERROR on closing the socket");

	return 0; 
}
