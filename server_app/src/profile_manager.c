#include "../lib/database.h"
#include "../lib/profile_manager.h"

int load_profiles(profile_list list) {
    return 0;
}

int get_profile(char *profile_name) {
    FILE *profiles;
    profiles = fopen("../data/profiles.db", "r");

    char profile[32];
    
    // Lendo o primeiro profile
    fscanf(profiles, "%[^,],", profile);
    // printf("%s\n", profile);
    if (strcmp(profile, profile_name) == 0) { 
        fclose(profiles);
        return 0;
    }
    // ..

    fclose(profiles);
    return -1;
}
