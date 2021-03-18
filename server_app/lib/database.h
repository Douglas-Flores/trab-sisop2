#include "profile_manager.h"

typedef struct __client_args {
    profile_list *profiles;     // Lista de perfis
    int sockfd;                 // Socket de comunicação
} client_args;