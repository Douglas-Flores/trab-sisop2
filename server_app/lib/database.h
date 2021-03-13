#include "com_manager.h"

typedef struct profile
{
    int user_id;
    char user_name[20];
};

typedef struct follower_list
{
    profile* follower;
    follower_list* next;
};

typedef struct notification_list
{
    notification* data;
    profile author;
    notification_list* next;
};


