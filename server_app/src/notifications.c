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
        
        sem_post(follower->inbox_sem);  // "produzindo" uma nova notificação para o usuário
        postinbox(follower, newnot);
        printf ("Notificação postada na caixa postal de %s\n", follower->username);
        
        fnode = fnode->next;
    }
    // ..
    
    return 0;
}

int postinbox(profile *receiver, notification *not) {
    notification_list *list;
    list = receiver->inbox;
    
    // Insere na inbox  SESSÃO CRÍTICA
    if(list->notification == NULL)
        list->notification = not;
    else {
        notification_list *newnode;
        newnode = malloc(sizeof(notification_list));
        newnode->notification = not;
        newnode->next = NULL;
        
        while (list->next != NULL)
            list = list->next;
        
        list->next = newnode;  
    }
    // ..   FIM DA SESSÃO CRÍTICA

    return 0;
}

int printinbox(profile *target) {
    notification_list *list;
    list = target->inbox;
        
    while (list != NULL) {
        printf("%s\n", list->notification->_string);
        list = list->next;
    }

    return 0;
}