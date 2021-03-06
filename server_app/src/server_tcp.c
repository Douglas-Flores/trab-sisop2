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
#include <signal.h>
#include "../lib/com_manager.h"
#include "../lib/database.h"
#include "../lib/notifications.h"
//#include "../lib/files.h"

#define PORT 4000

// SOMENTE PARA TESTES ------------------------------------------
#define handle_error_en(en, msg) \
  do { errno = en; perror(msg); exit(EXIT_FAILURE); } while (0)

#define handle_error(msg) \
  do { perror(msg); exit(EXIT_FAILURE); } while (0)
// -------------------------------------------------------------

profile_list *profiles;
pthread_mutex_t notthread;
pthread_cond_t authentication;
void INThandler(int);

int main(int argc, char *argv[]) {
	int sockfd, option = 1;
	struct sockaddr_in serv_addr;
	setbuf(stdout, NULL);
	
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
	init_notid(profiles);
	// ..

	signal(SIGINT, INThandler);
	// Loop de leitura por novas requisições de conexão
	if (listen(sockfd, BACKLOG_MAX) == 0) {
		int clisockfd, notifsockfd;

		while (2>1) {
			// Aceitando nova Conexão
			struct sockaddr_in cli_addr;
			socklen_t clilen = sizeof(struct sockaddr_in);
			if ((clisockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen)) == -1) {
				printf("ERROR on accept\n");
				continue;
			}
			// ..

			// Canal de notificações
			if ((notifsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen)) == -1) {
				printf("ERROR on accepting notifications channel\n");
				continue;
			}
			// ..

			// Criando thread para autenticação  
			pthread_t th;
			args->sockfd_1 = clisockfd;
			args->sockfd_2 = notifsockfd;
			int *result;
			pthread_create(&th, NULL, init_client, args);
			pthread_join(th, (void **) &result);
			// ..
			
			// Verificando resultado da autenticação
			if (*result < 0)
				close(notifsockfd);
			else {
				// Criando thread para interação com usuário
				pthread_t clithread;
				args->userid = *result;
				pthread_create(&clithread, NULL, client_thread, args);
				// ..
			}
			// ..

			clisockfd = 0;
			notifsockfd = 0;
		}
	}
	// ..

	if (close(sockfd) < 0)
    printf("ERROR on closing the socket");

	return 0; 
}

void *init_client(void *args) {
	int sockfd;
	int *userid;
	userid = malloc(sizeof(int));
	client_args *rargs = args;

	sockfd = rargs->sockfd_1;
	profiles = rargs->profiles;

	*userid = authenticate(sockfd, profiles);
	if (*userid < 0) {
		printf("Falha de Autenticacao.\n");
		close(sockfd);
	}
	
	pthread_exit(userid);
}

void *client_thread(void *args) {
	int n, sockfd, sockfd_n, userid;
	char buffer[BUFFER_SIZE];			// declaração do buffer
	profile *cur_user;					// perfil do usuário "dono" da thread
	client_args *rargs = args;			// argumentos
	session_t *cur_session;

	setbuf(stdout, NULL);				// setanto buffer da stdout para zero

	// Extração dos argurmentos
	sockfd = rargs->sockfd_1;
	sockfd_n = rargs->sockfd_2;
	profiles = rargs->profiles;
	userid = rargs->userid;
	// ..

	cur_user = get_profile_byid(profiles, userid);	// obtendo perfil do usuário
	
	if (cur_user->session_1.isopen == false) {
		cur_user->session_1.isopen = true;
		cur_user->session_1.cmdsockfd = sockfd;
		cur_user->session_1.nsockfd = sockfd_n;
		cur_session = &(cur_user->session_1);
	}
	else if (cur_user->session_2.isopen == false) {
		cur_user->session_2.isopen = true;
		cur_user->session_2.cmdsockfd = sockfd;
		cur_user->session_2.nsockfd = sockfd_n;
		cur_session = &(cur_user->session_2);;
	}
	else {
		cur_session = NULL;
	}

	// Criando nova thread para notificações
	pthread_t n_thread;
	printf("Session %d opened\n", cur_session->id);
	pthread_create(&n_thread, NULL, notification_thread, cur_session);
	// ..

	while(strcmp(buffer,"exit\n") != 0){

		// Lendo do Socket
		bzero(buffer, BUFFER_SIZE);
		n = read(sockfd, buffer, BUFFER_SIZE);
		if (n < 0) {
			printf("ERROR reading from socket\n");
			continue;
		}
		else if (buffer[0] == '\0' || buffer[0] == '\n'){
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
		else if(strcmp(cmd,"SEND") == 0)
			new_notification(profiles, cur_user, data, response);
		else
			strcpy(response, "Invalid command, try again...");
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

	cur_user->open_sessions--;
	cur_session->isopen = false;
	pthread_cancel(n_thread);
	close(sockfd_n);
	close(sockfd);
	printf("%s disconnected...\n", cur_user->username);

	return 0;
}

void *notification_thread(void *args) {
	int sockfd;
	profile *cur_user;
	session_t *session = args;			// argumentos
	setbuf(stdout, NULL);

	// Extração dos argurmentos
	sockfd = session->nsockfd;
	// ..

	cur_user = session->owner;			// obtendo perfil do usuário

	while (true)
	{

		printf("%s está esperando notificações.\n", cur_user->username);
		sem_wait(&(cur_user->inbox.full));		// esperando dados no buffer
		sem_wait(&(cur_user->inbox.mutexC));	// garantindo exclusão mútua no consumo

		// Obtendo a notificação mais recente
		notification *n;
		n = &((cur_user->inbox.inbox)[cur_user->inbox.front]);
		// ..

		// Criando pacote para enviar
		char *msg = malloc(sizeof(char)*148);
		strcat(msg,n->author);
		strcat(msg,": ");
		strcat(msg,n->_string);
		packet package;
		package.type = DATA;
		package.seqn = 0;
		package.timestamp = time(NULL);
		package._payload = msg;
		package.length = strlen(msg)+1;
		// ..

		char buffer[BUFFER_SIZE] = "\0";
		while (buffer[0] == '\0') {
			// Enviar mensagem
			printf("send %s para %s\n", msg, cur_user->username);
			if (send_packet(sockfd, &package) < 0)
				continue;
			// ..

			// Aguardando resposta
			int r = read(sockfd, buffer, BUFFER_SIZE);
			if (r < 0) {
				printf("Notificação de %s perdida.\n", cur_user->username);
				bzero(buffer, BUFFER_SIZE);
			}

			if (strcmp(buffer,"resend") == 0) {
				printf("Resposta3 %s\n", buffer);
				usleep(100000);
				bzero(buffer, BUFFER_SIZE);
				break;
			} 
			else if (strcmp(buffer,"received") == 0){
				printf("Resposta2 %s\n", buffer);
				bzero(buffer, BUFFER_SIZE);
				break;
			}
			else {
				printf("Resposta %s\n", buffer);
				usleep(100000);
				bzero(buffer, BUFFER_SIZE);
				break;
			} 
			// ..
		}

		// Caso haja mais de uma sessão aberta
		if (cur_user->open_sessions > 1) {
			bzero(buffer, BUFFER_SIZE);
			buffer[0] = '\0';

			if (session->id == 1)
				sockfd = cur_user->session_2.nsockfd;
			else
				sockfd = cur_user->session_1.nsockfd;
			
			while (buffer[0] == '\0') {
				// Enviar mensagem
				if (send_packet(sockfd, &package) < 0)
					break;
				// ..

				// Aguardando resposta
				int r = read(sockfd, buffer, BUFFER_SIZE);
				if (r < 0) {
					printf("ERROR reading from socket\n");
					bzero(buffer, BUFFER_SIZE);
					continue;
				}
				else if (strcpy(buffer,"received") != 0) {
					printf("Resposta não recebida, reenviando notificação...\n");
					break;
				}
				// ..
			}
			sockfd = session->nsockfd;
		}
		// ..

		free(msg);

		// Decrementando número de usuários pendentes
		profile *author = get_profile_byname(profiles, n->author);
		notification *p = get_notification_byid(author->notifications, n->id);
		if (p->pending > 0)
			p->pending = p->pending - 1;
		if (p->pending < 1)
			destroy_notification(author->notifications, p->id);
		// ..

		// Atualizando a inbox
		cur_user->inbox.front = (cur_user->inbox.front + 1) % INBOX_SIZE;
		// ..

		// Liberando semáforos
		sem_post(&(cur_user->inbox.mutexC));
		sem_post(&(cur_user->inbox.empty));
		// ..
	}
	
	printf("Fechando canal de notificações\n");

	pthread_exit(NULL);
}

void INThandler(int sig) {
	signal(sig, SIG_IGN);
	printf("\nSalvando status do banco de dados...\n");
	//save_profile_list(profiles);
	exit(0);
}
