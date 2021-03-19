#include <stdio.h>
#include <string.h>
#include "../lib/profile_manager.h"
#include <inttypes.h>




int write_to_file(int count, struct profile *data, char const *fileName)
{
    FILE *f = fopen(fileName, "w");
    if (f == NULL) return -1;
    {

     //fprintf(f, "%s,%s,%s\n", data->someValue, data->someString, data->someSample);
//        ++data;
    }
    fclose(f);
    return 0;
}

int write_to_file_one(struct profile_list *data, char const *fileName)
{
    return write_to_file(1, data, fileName);
}

char *followers_to_string(profile_list * followers) {
    if(followers->profile == NULL) {
        return "[]";
    }
    int numberOfFollowers = 1;
    profile_list * followersIterator = followers;
    printf("\naqui\n");
    while(followersIterator->next != NULL) {

        numberOfFollowers++;
        followersIterator =  followersIterator->next;
    }



    char* followersString = (char*) calloc(numberOfFollowers*23, sizeof(char));

    followersString[0] = '[';
    followersIterator = followers;
    strcat(followersString, followersIterator->profile->username);

    while(followersIterator->next != NULL) {
        followersIterator =  followersIterator->next;
        strcat(followersString, ",");
        strcat(followersString, followersIterator->profile->username);

    }
    strcat(followersString, "]");
    return followersString;
}

char *notification_to_string(notification * notification) {
    char idString[ 16 ];
    char timeStampString[ 16 ];
    char lengthString[ 16 ];
    char pendingString[ 16 ];

    sprintf(idString,"%"PRIu32, notification->id);
    sprintf(timeStampString,"%"PRIu32, notification->timestamp);
    sprintf(lengthString,"%"PRIu16, notification->length);
    sprintf(pendingString,"%"PRIu16, notification->pending);

    char* notificationString = calloc(66+ sizeof(notification->_string), sizeof(char));
    notificationString[0] = '[';
    strcat(notificationString, idString);
    strcat(notificationString, ",");
    strcat(notificationString, timeStampString);
    strcat(notificationString, ",");
    strcat(notificationString, lengthString);
    strcat(notificationString, ",");
    strcat(notificationString, notification->_string);
    strcat(notificationString, ",");
    strcat(notificationString, pendingString);
    strcat(notificationString, "]");
    return  notificationString;

}

char *notification_list_to_string(notification_list * notificationList) {
    if(notificationList->notification == NULL) {
        return "[]";
    }
    int numberOfNotifications = 1;
    notification_list * notificationIterator = notificationList;
    printf("\naqui\n");
    while(notificationIterator->next != NULL) {

        numberOfNotifications++;
        notificationIterator =  notificationIterator->next;
    }



    char** listStringNotifications = calloc(numberOfNotifications, sizeof(char*));


    notificationIterator = notificationList;
    listStringNotifications[0] = notification_to_string(notificationIterator->notification);
    int notificationNumericalIterator = 1;
    while(notificationIterator->next != NULL) {
        notificationIterator =  notificationIterator->next;
        listStringNotifications[notificationNumericalIterator] = notification_to_string(notificationIterator->notification);
        notificationNumericalIterator++;
    }
    int sizeOfString = 0;
    int i;
    for(i=0;i<numberOfNotifications;i++){
        sizeOfString += sizeof(listStringNotifications[i]);
    }
    char* finalString = calloc(sizeOfString+numberOfNotifications+2, sizeof(char));

    for(i=0;i<numberOfNotifications;i++){
        strcat(finalString, listStringNotifications[i]);
        strcat(finalString, ",");
    }
    strcat(finalString, "[]");

    for(i=0;i<numberOfNotifications;i++){
        free(listStringNotifications[i]);
    }
    free(listStringNotifications);



    return finalString;
}

int main() {
    profile_list* profileList = calloc(1, sizeof(profile_list));


    profileList->profile = calloc(1, sizeof(profile_list));

    profileList->profile->username[0] = 'c';
    profileList->profile->username[1] = '\0';
    profileList->next = calloc(1, sizeof(profile_list));
    profileList->next->profile = calloc(1, sizeof(profile_list));
    profileList->next->profile->username[0] = 'A';

    profileList->next->profile->username[1] = '\0';
    profileList->next->next = NULL;

    char* followersString = followers_to_string(profileList);
    printf("%s", followersString);
    printf("\naqui\n");

    notification* not = calloc(1, sizeof(notification));
    not->id = 1;
    not->timestamp = 2;
    not->_string = calloc(sizeof("3"), sizeof(char));
    not->_string = "3";
    not->length = 4;
    not->pending = 5;
    notification_list* notificationList = calloc(1, sizeof(notification_list));
    notificationList->notification = not;
    notificationList->next = calloc(1, sizeof(notification_list));
    notificationList->next->notification = not;
    notificationList->next->next=NULL;
    char* notString = notification_list_to_string(notificationList);
    printf("\n%s\n", notString);

}
