#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <time.h>

typedef struct __profile {
    char username[20];                          // Nome de usuário
    struct __profile_list *followers;           // Lista de seus seguidores
    struct __notification_list *notifications;  // Lista de seus 'tweets'
} profile;

typedef struct __profile_list {
    profile *profile;               // Perfil
    struct __profile_list *next;    // Próximo da lista
} profile_list;

typedef struct __notification {
    uint32_t id;            // Identificador da notificação (sugere-se um identificador único)
    uint32_t timestamp;     // Timestamp da notificação
    const char* _string;    // Mensagem
    uint16_t length;        // Tamanho da mensagem
    uint16_t pending;       // Quantidade de leitores pendentes
} notification;

typedef struct __notification_list {
    notification *notification;         // Notificação
    struct __notification_list *next;
} notification_list;


int load_profiles(profile_list *profiles);
int get_profile(char *profile_name);