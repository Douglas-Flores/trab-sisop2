#include <pthread.h>
#include <stdbool.h>
#include "profile_manager.h"

typedef struct __client_args {
    profile_list *profiles;     // Lista de perfis
    int sockfd_1;               // Socket primário
    int sockfd_2;               // Socket para canal de notificações
    int userid;                 // Id do usuário
} client_args;