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
#include "../lib/database.h"

#define PORT 4000

// SOMENTE PARA TESTES ------------------------------------------
#define handle_error_en(en, msg) \
  do { errno = en; perror(msg); exit(EXIT_FAILURE); } while (0)

#define handle_error(msg) \
  do { perror(msg); exit(EXIT_FAILURE); } while (0)
// -------------------------------------------------------------

profile_list *profiles;

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

  // Carregando estruturas de dados
  profiles = malloc(sizeof(profile_list));
  load_profiles(profiles);
  client_args *args = malloc(sizeof(args));
  args->profiles = profiles;
  // ..

  // Loop de leitura por novas requisições de conexão
  while (2>1) {  // TODO: condição de saida
    if (listen(sockfd, BACKLOG_MAX) == 0) {
      int newsockfd;
      socklen_t clilen;
      struct sockaddr_in cli_addr;

      // Abrindo um novo socket
      clilen = sizeof(struct sockaddr_in);
      if ((newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen)) == -1)
        printf("ERROR on accept\n");
      // ..

      // Criando thread para o novo usuário conectado  
      pthread_t th;
      args->sockfd = newsockfd;
      pthread_create(&th, NULL, client_thread, args);
      // ..
    }
  }
  // ..

	if (close(sockfd) < 0)
    printf("ERROR on closing the socket");

	return 0; 
}

void *client_thread(void *args) {
	
	int n, sockfd, userid;
	char buffer[BUFFER_SIZE];
	profile *cur_user;
	client_args *rargs = args;
	setbuf(stdout, NULL);

	sockfd = rargs->sockfd;
	profiles = rargs->profiles;

	userid = authenticate(sockfd, profiles);
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
		if(strcmp(cmd,"SEND") == 0)
      //printf("batata\n");
			new_notification(cur_user, data, response);
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
