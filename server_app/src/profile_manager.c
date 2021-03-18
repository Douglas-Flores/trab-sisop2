#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "../lib/profile_manager.h"

int load_profiles(profile_list *profiles) {
    FILE *db;
    db = fopen("../data/profiles.db", "r");

    profile_list *pnode = profiles;
    bool first_read = true;
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
        //printf("%s {\n", prof->username);
        // ..

        // Lendo lista de seguidores
        char buffer[256];
        profile_list *node = followers;
        fseek(db, 1, SEEK_CUR);
        bool first_follower = true;
        do {
            fscanf(db, "%[^,],", buffer);                   // Lendo em um buffer
            if (strcmp(buffer,"]") == 0)                    // Testando saída
                break;
            profile *follower = malloc(sizeof(profile));    // Alocando um novo seguidor
            strcpy(follower->username, buffer);             // Carregando username

            if (first_follower) {
                node->profile = follower;
                node->next = NULL;
                first_follower = false;
            } else {
                profile_list *fnode;
                fnode = malloc(sizeof(profile_list));
                fnode->profile = follower;
                fnode->next = NULL;
                node->next = fnode;
                node = fnode;
            }
            //printf("  %s,\n", node->profile->username);
        } while (strcmp(buffer,"]") != 0);
        // ..

        // Lendo lista de notificações
        notification_list *notnode = notifications;
        bzero(buffer, 256);

        // Lendo Primeira Notificação
        fseek(db, 1, SEEK_CUR);         // avançando um byte
        fscanf(db, "%[^,],", buffer);
        bool first_not = true;
        while (strcmp(buffer,"]") != 0) {
            //printf("  [");
            notification *not;
            not = malloc(sizeof(notification));

            not->id = atoi(buffer);         // lendo id
            //printf("%d, ", not->id);

            fscanf(db, "%[^,],", buffer);   // lendo timestamp
            not->timestamp = (time_t) atoi(buffer);
            //printf("%d, ", not->timestamp);

            fseek(db, 1 ,SEEK_CUR);
            fscanf(db, "%[^\"]\"", buffer); // lendo mensagem
            not->_string = malloc(sizeof(char)*(strlen(buffer)+1));
            strcpy(not->_string,buffer);
            //printf("\"%s\", ", not->_string);

            fseek(db, 1 ,SEEK_CUR);
            fscanf(db, "%[^,],", buffer);   // lendo tamanho da mensagem
            not->length = atoi(buffer);
            //printf("%d, ", not->length);

            fscanf(db, "%[^\],]\]", buffer);   // lendo quantidade de pendentes
            not->pending = atoi(buffer);
            //printf("%d]\n", not->pending);

            if(first_not) {
                notnode->notification = not;
                notnode->next = NULL;
                first_not = false;
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

        //printf("}\n");

        if (first_read) {
            pnode->profile = prof;
            pnode->next = NULL;
            first_read = false;
        } else {
            profile_list *newpnode;
            newpnode = malloc(sizeof(profile_list));
            newpnode->profile = prof;
            newpnode->next = NULL;
            pnode->next = newpnode;
            pnode = newpnode;
        }

    } while (true);

    fclose(db);
    return -1;
}

int get_profile(char *profile_name, profile_list *list) {

    profile_list *node = list;
    while (node != NULL)
    {
        if (strcmp(node->profile->username, profile_name) == 0)
            return 0;
        node = node->next;
    }

    return -1;
}
