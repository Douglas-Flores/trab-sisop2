#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "../lib/profiles.h"
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
        notification_list *inbox = malloc(sizeof(notification_list));
        inbox->notification = NULL;
        inbox->next = NULL;
        sem_t *inbox_sem = malloc(sizeof(sem_t));
        sem_init(inbox_sem, 0, 0);
        profile *prof = malloc(sizeof(profile));
        prof->open_sessions = 0;
        prof->followers = followers;
        prof->notifications = notifications;
        prof->inbox = inbox;
        prof->inbox_sem = inbox_sem;
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

int validate_profile(char *username, profile_list *list) {

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
	n = validate_profile(buffer, profiles);
	// ..

    // Obtendo perfil de usuário
    profile *user;
    if (n >= 0)
        user = get_profile_byid(profiles, n);
    else
        user = NULL;
    // ..

	// Criando pacote para enviar
	packet package;
	package.type = DATA;
	package.seqn = 0;
	package.timestamp = time(NULL);
	if (n >= 0 && user->open_sessions < MAX_SESSIONS) {
        user->open_sessions = user->open_sessions + 1;
		package._payload = "success";
		printf("%s logged\n", buffer);
	}
	else {
		package._payload = "failure";
        n = -1;
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

    n = validate_profile(username, profiles);
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

    profile *to_follow = get_profile_byname(profiles, username);
    profile_list *fnode = to_follow->followers;
    n = 0;
    int i = 0;
    while (fnode != NULL) {
        if(fnode->profile == NULL) { n = -1; break;}
            
        if (strcmp(fnode->profile->username, logged->username) == 0) {
            printf("Rejected: %s is already following %s\n", logged->username, username);
            strcpy(response, "Error! You are already following ");
            strcat(response, username);
            n = -1;
        }

        fnode = fnode->next;
        i++;
    }

    // Testando se ouve algum match no while anterior. Isso é importante por i precisa ser o tamanho da lista de seguidores
    if (n < 0) 
        return n;
    
    // Se chegou até aqui, pode seguir

    // Criando novo nodo
    profile *newfollower = malloc(sizeof(profile));
    strcpy(newfollower->username, logged->username);
    profile_list *newnode = malloc(sizeof(profile_list));
    newnode->profile = newfollower;
    newnode->next = NULL;
    // ..

    // Inserindo novo nodo na lista     Obs.: i = tamanho da lista de seguidores
    profile_list *followers = to_follow->followers;
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

int count_followers(profile *user) {
    int n = 0;
    profile_list *followers = user->followers;

    while (followers->profile != NULL) {
        n++;
        if (followers->next == NULL)
            break;
        else
            followers = followers->next;
    }
    
    return n;
}

profile* get_profile_byname(profile_list *list, char *username) {
    
    profile_list *node = list;
    while (node != NULL)
    {
        if (strcmp(node->profile->username, username) == 0)
            return node->profile;
        node = node->next;
    }

    return NULL;
}

profile* get_profile_byid(profile_list *list, int id) {
    
    profile_list *node = list;

    for (int i = 0; i < id; i++) {
        if (node->next != NULL)
            node = node->next;
        else {
            node = NULL;
            break;
        }
    }

    return node->profile;
}