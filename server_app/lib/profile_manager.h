#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <time.h>
#include "notification.h"

#ifndef _PROFILEMANAGER_
#define _PROFILEMANAGER_

typedef struct __profile {
    char username[20];                          // Nome de usuário
    struct __profile_list *followers;           // Lista de seus seguidores
    struct __notification_list *notifications;  // Lista de seus 'tweets'
} profile;

typedef struct __profile_list {
    profile *profile;               // Perfil
    struct __profile_list *next;    // Próximo da lista
} profile_list;

int load_profiles(profile_list *profiles);
int get_profile(char *username, profile_list *list);
int authenticate(int socket, profile_list *profiles);
int follow(profile_list *profiles, profile *logged, char *username, char *response);


#endif