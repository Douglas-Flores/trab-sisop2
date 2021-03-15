#include <stdint.h>
#include <time.h>

#define DATA 1
#define CMD 2
#define BACKLOG_MAX 10
#define BUFFER_SIZE 256

typedef struct __notification {
    uint32_t id;            // Identificador da notificação (sugere-se um identificador único)
    uint32_t timestamp;     // Timestamp da notificação
    const char* _string;    // Mensagem
    uint16_t length;        // Tamanho da mensagem
    uint16_t pending;       // Quantidade de leitores pendentes
} notification;

typedef struct __packet {
    uint16_t type;          //Tipo do pacote (p.ex. DATA | CMD)
    uint16_t seqn;          //Número de sequência
    uint16_t length;        //Comprimento do payload 
    time_t timestamp;       // Timestamp do dado
    const char* _payload;   //Dados da mensagem
} packet;

void *client_thread(void *sockfd);
int read_from_socket();
int write_on_socket();
int send_packet(int socket, packet *package);