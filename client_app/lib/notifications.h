/*
    Author: Douglas Souza Flôres
    Biblioteca para manipulação de notificações
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <time.h>

#ifndef _NOTIFICATIONS_H
#define _NOTIFICATIONS_H

//  STRUCTS
typedef struct __notification {
    uint32_t id;            // Identificador da notificação (sugere-se um identificador único)
    char author[20];        // Nome do autor
    uint32_t timestamp;     // Timestamp da notificação
    char* _string;          // Mensagem
    uint16_t length;        // Tamanho da mensagem
    uint16_t pending;       // Quantidade de leitores pendentes
} notification;

typedef struct __notification_list {
    notification *notification;         // Notificação
    struct __notification_list *next;
} notification_list;

#endif