#include "../lib/files.h"
#include "../lib/profiles.h"

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

    if(followers == NULL) {
        return "[]";
    }
    int numberOfFollowers = 1;
    profile_list * followersIterator = followers;

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

    char* notificationString = calloc(66+ strlen(notification->_string), sizeof(char));
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
    char* finalString = malloc(sizeof(char)*256);
    
    if(notificationList == NULL) {
        return "[]";
    }
    int numberOfNotifications = 1;
    notification_list * notificationIterator = notificationList;

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
        sizeOfString += strlen(listStringNotifications[i]);
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


char* profile_to_string(profile currentProfile){
    char* followersString = followers_to_string(currentProfile.followers);
    char* notificationString = notification_list_to_string(currentProfile.notifications);
    char* profileString = calloc(strlen(followersString)+ strlen(notificationString)+ strlen(currentProfile.username)+3,
                                 sizeof(char));

    strcat(profileString, currentProfile.username);
    strcat(profileString, ",");
    strcat(profileString, followersString);
    strcat(profileString, ",");

    strcat(profileString, notificationString);
    return notificationString;
}

void save_profile_list(profile_list * profileList) {
    if(profileList == NULL) {
        return;
    }
    int numberOfProfiles = 1;
    profile_list *profileIterator = profileList;

    while(profileIterator->next != NULL) {

        numberOfProfiles++;
        profileIterator =  profileIterator->next;
    }


    FILE* file  = fopen("../data/profiles2.db", "w+");


    profileIterator = profileList;
    char* profileString = profile_to_string(*profileIterator->profile);
    fprintf(file, profileString);
    free(profileString);

    while(profileIterator->next != NULL) {

        profileIterator =  profileIterator->next;
        printf(".\n");
        char* profileString = profile_to_string(*profileIterator->profile);
        fprintf(file, profileString);
        free(profileString);
    }
    fclose(file);
}

