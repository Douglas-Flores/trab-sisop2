#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <time.h>
#include <semaphore.h>
#include "notifications.h"

#ifndef _PROFILEMANAGER_
#define _PROFILEMANAGER_
#define MAX_SESSIONS 2
#define INBOX_SIZE 16

typedef struct __inbox {
    struct __notification inbox[INBOX_SIZE];    // Array de notificações
    sem_t empty;                                // Semáforo empty
    sem_t full;                                 // Semáforo full
    sem_t mutexP;                               // Mutex dos produtores
    sem_t mutexC;                               // Mutex dos consumidores
    int front;
    int rear;
} inbox;

typedef struct __profile {
    char username[20];                          // Nome de usuário
    int open_sessions;                          // Número de sessões abertas
    struct __profile_list *followers;           // Lista de seus seguidores
    struct __notification_list *notifications;  // Lista de seus 'tweets'
    struct __inbox inbox;                      // Lista de notificações pendentes
} profile;

typedef struct __profile_list {
    profile *profile;               // Perfil
    struct __profile_list *next;    // Próximo da lista
} profile_list;

void print_profile_list(profile_list *list);
int load_profiles(profile_list *profiles);
profile* get_profile_byname(profile_list *list, char *username);
profile* get_profile_byid(profile_list *list, int id);
int validate_profile(char *username, profile_list *list);
int authenticate(int socket, profile_list *profiles);
int follow(profile_list *profiles, profile *logged, char *username, char *response);
int count_followers(profile *user);

#endif