#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../lib/database.h"
#include "../lib/profile_manager.h"

int load_profiles(profile_list *profiles) {
    FILE *db;
    db = fopen("../data/profiles.db", "r");

    profile_list *pnode = profiles;
    do {
        // Iniciando variáveis
        profile_list *followers = malloc(sizeof(profile_list));
        followers->next = NULL;
        notification_list *notifications = malloc(sizeof(notification_list));
        notifications->next = NULL;
        profile *prof = malloc(sizeof(profile));
        prof->followers = followers;
        prof->notifications = notifications;
        // ..

        // Lendo username
        if (fscanf(db, "%[^\[]", prof->username) == EOF)
            break;
        printf("%s {\n", prof->username);
        // ..

        // Lendo lista de seguidores
        char buffer[256];
        profile_list *node = followers;
        fseek(db, 1, SEEK_CUR);
        do {
            fscanf(db, "%[^,],", buffer);                   // Lendo em um buffer
            if (strcmp(buffer,"]") == 0)                    // Testando saída
                break;
            profile *follower = malloc(sizeof(profile));    // Alocando um novo seguidor
            strcpy(follower->username, buffer);             // Carregando username

            if (node == followers) {
                node->profile = follower;
                node->next = NULL;
            } else {
                profile_list *fnode;
                fnode = malloc(sizeof(profile_list));
                fnode->profile = follower;
                fnode->next = NULL;
                node->next = fnode;
                node = fnode;
            }
            printf("  %s,\n", node->profile->username);
        } while (strcmp(buffer,"]") != 0);
        // ..

        // Lendo lista de notificações
        notification_list *notnode = notifications;
        bzero(buffer, 256);

        // Lendo Primeira Notificação
        fseek(db, 1, SEEK_CUR);         // avançando um byte
        fscanf(db, "%[^,],", buffer);
        while (strcmp(buffer,"]") != 0) {
            printf("  [");
            notification *not;
            not = malloc(sizeof(notification));

            not->id = atoi(buffer);         // lendo id
            printf("%d, ", not->id);

            fscanf(db, "%[^,],", buffer);   // lendo timestamp
            not->timestamp = (time_t) atoi(buffer);
            printf("%d, ", not->timestamp);

            fseek(db, 1 ,SEEK_CUR);
            fscanf(db, "%[^\"]\"", buffer); // lendo mensagem
            not->_string = malloc(sizeof(char)*(strlen(buffer)+1));
            strcpy(not->_string,buffer);
            printf("\"%s\", ", not->_string);

            fseek(db, 1 ,SEEK_CUR);
            fscanf(db, "%[^,],", buffer);   // lendo tamanho da mensagem
            not->length = atoi(buffer);
            printf("%d, ", not->length);

            fscanf(db, "%[^\],]\]", buffer);   // lendo quantidade de pendentes
            not->pending = atoi(buffer);
            printf("%d]\n", not->pending);

            if(notnode == notifications) {
                notnode->notification = not;
                notnode->next = NULL;
            } else {
                notification_list *newnode;
                newnode = malloc(sizeof(notification_list));
                newnode->notification = not;
                newnode->next = NULL;
                notnode->next = newnode;
                notnode = newnode;
            }

            // preparando-se para a próxima iteração
            fseek(db, 2, SEEK_CUR);
            fscanf(db, "%[^,],", buffer);
        }
        // ..

        printf("}\n");

        if (pnode == profiles) {
            pnode->profile = prof;
            pnode->next = NULL;
        } else {
            profile_list *newpnode;
            newpnode = malloc(sizeof(profile_list));
            newpnode->profile = prof;
            newpnode->next = NULL;
            pnode->next = newpnode;
            newpnode = newpnode;
        }

    } while (5>1);

    fclose(db);
    return -1;
}

int get_profile(char *profile_name) {
    FILE *profiles;
    profiles = fopen("../data/profiles.db", "r");

    char profile[32];
    
    // Lendo o primeiro profile
    fscanf(profiles, "%[^,],", profile);
    // printf("%s\n", profile);
    if (strcmp(profile, profile_name) == 0) { 
        fclose(profiles);
        return 0;
    }
    // ..

    fclose(profiles);
    return -1;
}
