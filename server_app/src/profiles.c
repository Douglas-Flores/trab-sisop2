/*
    Author: Douglas Souza Flôres
    Biblioteca para manipulação de perfis e sessões de uso
*/

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
        profile *prof = malloc(sizeof(profile));

        profile_list *followers = malloc(sizeof(profile_list));
        followers->profile = NULL;
        followers->next = NULL;
        
        notification_list *notifications = malloc(sizeof(notification_list));
        notifications->notification = NULL;
        notifications->next = NULL;
        
        inbox inbox;
        sem_t *empty = malloc(sizeof(sem_t));
        sem_init(empty, 0, INBOX_SIZE);
        sem_t *full = malloc(sizeof(sem_t));
        sem_init(full, 0, 0);
        sem_t *mutexP = malloc(sizeof(sem_t));
        sem_init(mutexP, 0, 1);
        sem_t *mutexC = malloc(sizeof(sem_t));
        sem_init(mutexC, 0, 1);
        inbox.empty = *empty;
        inbox.full = *full;
        inbox.mutexP = *mutexP;
        inbox.mutexC = *mutexC;
        inbox.front = 0;
        inbox.rear = 0;
        
        session_t session_1;
        session_1.id = 1;
        session_1.isopen = false;
        session_1.cmdsockfd = 0;
        session_1.nsockfd = 0;
        session_1.owner = prof;

        session_t session_2;
        session_2.id = 2;
        session_2.isopen = false;
        session_2.cmdsockfd = 0;
        session_2.nsockfd = 0;
        session_2.owner = prof;

        prof->open_sessions = 0;
        prof->session_1 = session_1;
        prof->session_2 = session_2;
        prof->followers = followers;
        prof->notifications = notifications;
        prof->inbox = inbox;
        // ..

        // Lendo username
        if (fscanf(db, "%[^\[]", prof->username) == EOF)
            break;
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

            strcpy(not->author, prof->username);

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
    return 0;
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
    else {
        user = create_new_profile(profiles, buffer);
        n = validate_profile(buffer, profiles);
    }
    // ..

	// Criando pacote para enviar
	packet package;
	package.type = DATA;
	package.seqn = 0;
	package.timestamp = time(NULL);
	if (n >= 0 && user->open_sessions < MAX_SESSIONS) {
        user->open_sessions = user->open_sessions + 1;
		package._payload = "success";
		printf("%s logged\n", user->username);
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
    if (fnode->profile != NULL)
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

void print_profile_list(profile_list *list) {
    profile_list *node = list;
    profile *prof;
    int *s1 = malloc(sizeof(int));

    while (node != NULL)
    {
        prof = node->profile;
        printf("%s {\n", prof->username);
        printf("  open sessions: %d\n", prof->open_sessions);
        sem_getvalue(&(prof->inbox.empty), s1);
        printf("  empty: %d\n", *s1);
        sem_getvalue(&(prof->inbox.full), s1);
        printf("  full: %d\n", *s1);
        sem_getvalue(&(prof->inbox.mutexP), s1);
        printf("  mutexP: %d\n", *s1);
        printf("  front: %d\n", prof->inbox.front);
        printf("  rear: %d\n", prof->inbox.rear);
        printf("  followers: \n");
        profile_list *flw = prof->followers;
        while (flw != NULL) {
            printf("    %s\n", flw->profile->username);
            flw = flw->next;
        }
        printf("  notifications: \n");
        notification_list *not = prof->notifications;
        while (not != NULL) {
            if(not->notification == NULL)
                break;
            printf("    [");
            printf("%d, ",not->notification->id);
            printf("%s, ",not->notification->author);
            printf("%d, ",not->notification->timestamp);
            printf("%s, ",not->notification->_string);
            printf("%d, ",not->notification->length);
            printf("%d",not->notification->pending);
            printf("]\n");
            not = not->next;
        }
        
        
        printf("}\n");

        node = node->next;
    }

    free(s1);
}

profile* create_new_profile(profile_list *profiles, char *username) {
	profile_list *pnode = profiles;

    // Criando novo perfil
    profile *prof = malloc(sizeof(profile));

    profile_list *followers = malloc(sizeof(profile_list));
    followers->profile = NULL;
    followers->next = NULL;
    notification_list *notifications = malloc(sizeof(notification_list));
    notifications->notification = NULL;
    notifications->next = NULL;

    inbox inbox;
    sem_t *empty = malloc(sizeof(sem_t));
    sem_init(empty, 0, INBOX_SIZE);
    sem_t *full = malloc(sizeof(sem_t));
    sem_init(full, 0, 0);
    sem_t *mutexP = malloc(sizeof(sem_t));
    sem_init(mutexP, 0, 1);
    sem_t *mutexC = malloc(sizeof(sem_t));
    sem_init(mutexC, 0, 1);
    inbox.empty = *empty;
    inbox.full = *full;
    inbox.mutexP = *mutexP;
    inbox.mutexC = *mutexC;
    inbox.front = 0;
    inbox.rear = 0;

    session_t session_1;
    session_1.id = 1;
    session_1.isopen = false;
    session_1.cmdsockfd = 0;
    session_1.nsockfd = 0;
    session_1.owner = prof;

    session_t session_2;
    session_2.id = 2;
    session_2.isopen = false;
    session_2.cmdsockfd = 0;
    session_2.nsockfd = 0;
    session_2.owner = prof;

    prof->open_sessions = 0;
    prof->session_1 = session_1;
    prof->session_2 = session_2;
    prof->followers = followers;
    prof->notifications = notifications;
    prof->inbox = inbox;
    strcpy(prof->username, username);
    // ..

    // Inserindo profile na lista
	if (profiles->profile == NULL)
        pnode->profile = prof;
    else {
		while (pnode->next != NULL)
			pnode = pnode->next;
        
        profile_list *newnode = malloc(sizeof(profile_list));
        newnode->profile = prof;
        newnode->next = NULL;
        pnode->next = newnode;
    }
	// ..

    print_profile_list(profiles);

    return prof;
}

int print_inbox(inbox *inbox) {
    notification *box;
    box = inbox->inbox;

    int i = 0;
    while (i < inbox->rear)
    {
        printf("Message %i: %s\n", i, box[i]._string);
        i++;
    }
    
    return 0;
}