#include "com_manager.h"

typedef struct profile
{
    int user_id;
    char user_name[20];
} profile;

typedef struct follower_list
{
    profile* follower;
    struct follower_list* next;
} follower_list;

typedef struct notification_list
{
    notification* data;
    profile author;
    struct notification_list* next;
} notification_list;


