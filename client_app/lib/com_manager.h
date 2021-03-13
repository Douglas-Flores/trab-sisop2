#include <stdint.h>

#define DATA 1
#define CMD 2

typedef struct __packet {
    uint16_t type;          //Tipo do pacote (p.ex. DATA | CMD)
    uint16_t seqn;          //Número de sequência
    uint16_t length;        //Comprimento do payload 
    uint16_t timestamp;     // Timestamp do dado
    const char* _payload;   //Dados da mensagem
} packet;

int connect_to_server(char *end, char *port);