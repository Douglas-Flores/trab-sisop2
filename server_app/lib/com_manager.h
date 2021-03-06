#include <stdint.h>
#include <time.h>

#define DATA        1
#define CMD         2
#define BACKLOG_MAX 10
#define BUFFER_SIZE 256

#ifndef _COMMUNICATION_
#define _COMMUNICATION_

typedef struct __packet {
    uint16_t type;          //Tipo do pacote (p.ex. DATA | CMD)
    uint16_t seqn;          //Número de sequência
    uint16_t length;        //Comprimento do payload 
    time_t timestamp;       //Timestamp do dado
    char* _payload;   //Dados da mensagem
} packet;

void *notification_thread(void *args);
void *client_thread(void *args);
void *init_client(void *args);
int read_packet(int socket, packet *package, char *buffer);
int read_text(int socket, char *buffer);
int send_packet(int socket, packet *package);

#endif