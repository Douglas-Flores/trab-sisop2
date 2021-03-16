#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct __profile_list {
    char *profile;
    struct __profile_list *next;
} profile_list;

int load_profiles(profile_list list);
int get_profile(char *profile_name);