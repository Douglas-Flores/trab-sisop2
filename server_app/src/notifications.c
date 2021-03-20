#include "../lib/notifications.h"

int notid = 0;

int new_notification(profile *author, char* msg, char *response) {
    notification_list *list = author->notifications;
    char *_string = malloc(sizeof(char)*128);
    strcpy(_string,msg);

    // Criando nova notificação
    notification *newnot;
    newnot = malloc(sizeof(notification));
    newnot->id = notid;
    newnot->timestamp = time(NULL);              // TODO
    newnot->_string = _string;
    newnot->length = strlen(msg);
    newnot->pending = count_followers(author);
    // ..
    
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

    // Increamentando id
    notid++;
    
    return 0;
}