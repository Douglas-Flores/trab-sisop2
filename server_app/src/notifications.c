#include "../lib/notifications.h"

int notid = 0;

int new_notification(profile_list* profiles, profile *author, char* msg, char *response) {
    notification_list *list = author->notifications;
    char *_string = malloc(sizeof(char)*128);
    strcpy(_string,msg);

    // Criando nova notificação
    notification *newnot;
    newnot = malloc(sizeof(notification));
    // SESSÃO CRÍTICA
    newnot->id = notid;
    notid++;
    // FIM DA SESSÃO CRÍTICA
    strcpy(newnot->author, author->username);
    newnot->timestamp = time(NULL);
    newnot->_string = _string;
    newnot->length = strlen(msg);
    newnot->pending = count_followers(author);
    // ..
    
    // Inserindo notificação na lista do autor  SESSÃO CRÍTICA
    if(list->notification == NULL) {
        list->notification = newnot;
        strcpy(response, "Message stored successfully!");
    }
    else {
        // Criando novo nodo
        notification_list *newnode = malloc(sizeof(notification_list));
        newnode->notification = newnot;
        newnode->next = NULL;
        // ..

        // Percorrendo a lista até o final
        while (list->next != NULL) {
            list = list->next;
        }
        // ..

        // Adicionando novo nodo
        list->next = newnode;

        strcpy(response, "Message stored successfully!");
    }
    // ..   FIM DA SESSÃO CRÍTICA

    // Enviando/postando notificações
    profile_list *fnode;
    fnode = author->followers;
    profile *follower;
    
    // Se não existem seguidores, essa etapa pode ser pulada
    if(fnode->profile == NULL)
        return 0;
    // ..

    while (fnode != NULL) {
        // Obtendo perfil do seguidor
        follower = get_profile_byname(profiles, fnode->profile->username);
        //printf("follower: %s / %d open sessions\n", follower->username, follower->open_sessions);

        /*int *s1 = malloc(sizeof(int));
        sem_getvalue(&(follower->inbox->empty), s1);
        printf("%s, empty (%d)\n", follower->username, *s1);
        free(s1);*/
        
        // "Produzindo" uma nova notificação para o usuário
        sem_wait(&(follower->inbox.empty));    // decrementando empty
        postinbox(follower, newnot);            // produzindo
        sem_post(&(follower->inbox.full));     // incrementando full
        printf ("Notificação postada na caixa postal de %s\n", follower->username);
        // ..

        fnode = fnode->next;
    }
    // ..
    
    return 0;
}

int postinbox(profile *receiver, notification *not) {
    inbox *ptr_inbox;
    ptr_inbox = &(receiver->inbox);
    notification *inbox = ptr_inbox->inbox;

    printf("MESSAGE %s rear: %d\n", not->_string, ptr_inbox->rear);
    int n = ptr_inbox->rear;
    printf("MESSAGE %s rear: %d\n", not->_string, n);
    // Insere na inbox 
    inbox[n] = *not;
    ptr_inbox->rear = (ptr_inbox->rear+1) % INBOX_SIZE;
    // ..

    return 0;
}

int printinbox(profile *target) {
    notification *inbox;
    inbox = target->inbox.inbox;
    
    int i = 0;
    while (inbox[i]._string != NULL) {
        printf("%s\n", inbox[i]._string);
        i++;
    }

    return 0;
}

notification* get_notification_byid(notification_list *list, int id) {
    
    notification_list *node = list;

    if(node->notification == NULL)
        return NULL;
    
    while (node != NULL) {
        
        if(node->notification->id == id)
            break;
        
        node = node->next;
    }

    return node->notification;
}