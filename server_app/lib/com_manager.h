#include <stdint.h>
#include <time.h>

#define DATA 1
#define CMD 2
#define BACKLOG_MAX 10
#define BUFFER_SIZE 256

typedef struct __packet {
    uint16_t type;          //Tipo do pacote (p.ex. DATA | CMD)
    uint16_t seqn;          //Número de sequência
    uint16_t length;        //Comprimento do payload 
    time_t timestamp;       //Timestamp do dado
    const char* _payload;   //Dados da mensagem
} packet;

void *client_thread(void *args);
int read_packet(int socket, packet *package, char *buffer);
int send_packet(int socket, packet *package);
int authenticate(int socket);