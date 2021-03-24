#include <stdint.h>
#include <time.h>

#define DATA 1
#define CMD 2
#define BUFFER_SIZE 256

typedef struct __packet {
    uint16_t type;          //Tipo do pacote (p.ex. DATA | CMD)
    uint16_t seqn;          //Número de sequência
    uint16_t length;        //Comprimento do payload 
    time_t timestamp;     // Timestamp do dado
    const char* _payload;   //Dados da mensagem
} packet;

typedef struct __client_thread_args {
    int sockfd;                 // Socket de comunicação
} client_thread_args;

void *cmd_routine(void *args);
void *notif_routine(void *args);
int connect_to_server(char *end, char *port);
int read_packet(int socket, packet *package, char *buffer);
int send_packet(int socket, packet *package);
int authenticate(int socket, char *username);