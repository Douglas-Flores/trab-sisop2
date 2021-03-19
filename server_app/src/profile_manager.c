#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "../lib/profile_manager.h"
#include "../lib/com_manager.h"

int load_profiles(profile_list *profiles) {
    FILE *db;
    db = fopen("../data/profiles.db", "r");

    profile_list *pnode = profiles;
    bool first_read = true;
    do {
        // Iniciando variáveis
        profile_list *followers = malloc(sizeof(profile_list));
        followers->profile = NULL;
        followers->next = NULL;
        notification_list *notifications = malloc(sizeof(notification_list));
        notifications->notification = NULL;
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

int get_profile(char *username, profile_list *list) {

    profile_list *node = list;
    int n = 0;
    while (node != NULL)
    {
        if (strcmp(node->profile->username, username) == 0)
            return n;
        node = node->next;
        n++;
    }

    return -1;
}

int authenticate(int socket, profile_list *profiles){
	int n;
	char buffer[BUFFER_SIZE];

	// Lendo username
	bzero(buffer, BUFFER_SIZE);
	n = read_text(socket, buffer);
	// ..

	// Verificando existência do usuário
	n = get_profile(buffer, profiles);
	// ..

	// Criando pacote para enviar
	packet package;
	package.type = DATA;
	package.seqn = 0;
	package.timestamp = time(NULL);
	if (n >= 0) {
		package._payload = "success";
		printf("%s logged\n", buffer);
	}
	else {
		package._payload = "failure";
		printf("%s attempted to log but was unsuccessful.\n", buffer);
	}
	package.length = strlen(package._payload) + 1;
	// ..

	// Enviando resposta
	send_packet(socket, &package);
	// ..

	return n;
}

int follow(profile_list *profiles, profile *logged, char *username, char *response) {
    int n = 0;
    bzero(response, 256);

    n = get_profile(username, profiles);
    if (n < 0) {
        printf("Rejected: %s attempted to follow an user that does not exist.\n", logged->username);
        strcpy(response, "Error! The user couldn't be found.");
        return n;
    }

    if (strcmp(logged->username, username) == 0) {
        printf("Rejected: %s attempted to follow itself.\n", username);
        strcpy(response, "Error! You can't follow yourself.");
        return -1;
    }

    profile_list *node = logged->followers;
    int i = 0;
    while (node != NULL)
    {
        if (strcmp(node->profile->username, username) == 0) {
            printf("Rejected: %s is already following %s\n", logged->username, username);
            strcpy(response, "Error! You are already following ");
            strcat(response, username);
            n = -1;
        }
        node = node->next;
        i++;
    }

    // Testando se ouve algum match no while anterior. Isso é importante por i precisa ser o tamanho da lista de seguidores
    if (n < 0) 
        return n;
    
    // Se chegou até aqui, pode seguir

    // Criando novo nodo
    profile *newfollower = malloc(sizeof(profile));
    strcpy(newfollower->username, username);
    profile_list *newnode = malloc(sizeof(profile_list));
    newnode->profile = newfollower;
    newnode->next = NULL;
    // ..

    // Inserindo novo nodo na lista     Obs.: i = tamanho da lista de seguidores
    profile_list *followers = logged->followers;
    if (followers == NULL) {
        free(newnode);
        followers->profile = newfollower;
    } else {
        for(int ptr = 1; ptr < i; ptr++)
            followers = followers->next;
        followers->next = newnode;
    }
    // ..

    strcpy(response, "Success! You are now following ");
    strcat(response, username);

    return n;
}