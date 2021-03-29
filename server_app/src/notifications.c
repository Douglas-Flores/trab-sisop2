/*
    Author: Douglas Souza Flôres
    Biblioteca para manipulação de notificações
*/

#include <semaphore.h>
#include "../lib/notifications.h"

extern int notid = 0;
sem_t sem_id, author_not;

int new_notification(profile_list* profiles, profile *author, char* msg, char *response) {
    notification_list *list = author->notifications;
    char *_string = malloc(sizeof(char)*128);
    strcpy(_string,msg);

    // Criando nova notificação
    notification *newnot;
    newnot = malloc(sizeof(notification));
    // SESSÃO CRÍTICA
    sem_wait(&sem_id);
    newnot->id = notid;
    notid++;
    sem_post(&sem_id);
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
    }

    strcpy(response, "Message stored successfully!");
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
        
        // "Produzindo" uma nova notificação para o usuário
        sem_wait(&(follower->inbox.empty));     // decrementando empty
        sem_wait(&(follower->inbox.mutexP));    // garantindo exclusão mútua na escrita
        postinbox(follower, newnot);            // produzindo
        sem_post(&(follower->inbox.mutexP));    // liberando lock de exclusão mútua
        sem_post(&(follower->inbox.full));      // incrementando full
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

    // Insere na inbox 
    int n = ptr_inbox->rear;
    inbox[n] = *not;
    ptr_inbox->rear = (n + 1) % INBOX_SIZE;
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

int init_notid(profile_list *list) {
    profile_list *pnode = list;
    sem_init(&sem_id, 0, 1);
    
    while (pnode != NULL) {
        notification_list *nnode = pnode->profile->notifications;

        while (nnode != NULL && nnode->notification != NULL) {
            
            if (nnode->notification->id >= notid)
                notid = nnode->notification->id + 1;
            
            nnode = nnode->next;
        }

        pnode = pnode->next;
    }

    return 0;
}

int destroy_notification(notification_list *list, int id) {
    notification_list *nnode = list;
    notification_list *prev_node = NULL;
    notification_list *next_node = NULL;
    bool match = false;

    if(nnode->notification == NULL)
        return -1;
    
    while (nnode != NULL) {
    
        next_node = nnode->next;
        
        if(nnode->notification->id == id) {
            match = true;
            break;
        }
        
        prev_node = nnode;
        nnode = nnode->next;
    }

    if (!match) { return -1; }

    if (prev_node == NULL) {
        // o nodo a ser excluído é a cabeça da lista
        if (next_node == NULL) {
            // existe somente um nodo na lista
            nnode->notification = NULL;
        }
        else {
            // excluindo a cabeça
            list = next_node;
            free(nnode);
        }
    }
    else if (next_node == NULL) {
        // o nodo a ser excluído é o rabo da lista
        prev_node->next = NULL;
        free(nnode);
    }
    else {
        // o nodo a ser exluído está no meio da lista
        prev_node->next = next_node;
        free(nnode);
    }
    
    return 0;
}