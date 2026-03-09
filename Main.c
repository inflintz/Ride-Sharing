#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_STR 128

typedef struct {
    char name[MAX_STR];
    int historyCount;
    char **destinations;
} UserHistory;

typedef struct {
    char name[MAX_STR];
    char travelDate[MAX_STR];     // DD-MM-YYYY
    char source[MAX_STR];
    char destination[MAX_STR];
    char departTime[MAX_STR];
} Ride;

void cleanField(char *s) {
    while (*s == ' ' || *s == '\"') s++; // remove leading spaces/quotes
    char *p = strchr(s, '\n');
    if (p) *p = '\0';
    int len = strlen(s);
    while (len > 0 && (s[len-1] == ' ' || s[len-1] == '\"')) s[--len] = '\0'; // trailing spaces/quotes
}

void splitPlaceFix(const char *place, char *src, char *dst) {
    // Handles both "VITC to X" and "X to VITC"
    char temp[MAX_STR];
    strncpy(temp, place, MAX_STR-1);
    temp[MAX_STR-1] = '\0';
    char *tok = strstr(temp, " to ");
    if (!tok) { strcpy(src, ""); strcpy(dst, temp); return; }
    *tok = 0;
    strcpy(src, temp);
    strcpy(dst, tok+4);
    cleanField(src);
    cleanField(dst);
}

void formatDate(const char *in, char *out) {
    // Handles "YYYY-MM-DD" to "DD-MM-YYYY" and "DD-MM-YYYY" passthrough
    if (strlen(in) == 10 && in[4] == '-' && in[7] == '-') {
        // YYYY-MM-DD to DD-MM-YYYY
        snprintf(out, MAX_STR, "%c%c-%c%c-%c%c%c%c",
            in[8], in[9], in[5], in[6], in[0], in[1], in[2], in[3]);
    } else if (strlen(in) == 10 && in[2] == '-' && in[5] == '-') {
        strncpy(out, in, MAX_STR-1);
        out[MAX_STR-1] = '\0';
    } else {
        strncpy(out, in, MAX_STR-1);
        out[MAX_STR-1] = '\0';
    }
}

int findOrAddUser(UserHistory **users, int *userCount, int *userCap, const char *name) {
    for (int i = 0; i < *userCount; i++)
        if (strcmp((*users)[i].name, name) == 0)
            return i;
    if (*userCount == *userCap) {
        *userCap *= 2;
        *users = realloc(*users, (*userCap) * sizeof(UserHistory));
    }
    strcpy((*users)[*userCount].name, name);
    (*users)[*userCount].historyCount = 0;
    (*users)[*userCount].destinations = NULL;
    return (*userCount)++;
}

void loadRideHistory(const char *fname, UserHistory **users, int *userCount, int *userCap) {
    FILE *fp = fopen(fname, "r");
    if (!fp) { printf("Could not open %s\n", fname); exit(1); }
    char line[512];
    fgets(line, sizeof(line), fp);

    while (fgets(line, sizeof(line), fp)) {
        char *fields[8] = {0}; int col = 0;
        char temp[512]; strncpy(temp, line, sizeof(temp));
        char *token = strtok(temp, ",");
        while (token && col < 8) fields[col++] = token, token = strtok(NULL, ",");

        if (col < 6 || !fields[2] || !fields[5]) continue;
        char name[MAX_STR], dest[MAX_STR], dummy[MAX_STR];
        strncpy(name, fields[2], MAX_STR-1); cleanField(name);
        char place[MAX_STR]; strncpy(place, fields[5], MAX_STR-1); cleanField(place);
        splitPlaceFix(place, dummy, dest);

        int idx = findOrAddUser(users, userCount, userCap, name);
        UserHistory *u = &((*users)[idx]);
        u->destinations = realloc(u->destinations, (u->historyCount + 1) * sizeof(char *));
        u->destinations[u->historyCount] = malloc(MAX_STR);
        strncpy(u->destinations[u->historyCount], dest, MAX_STR-1);
        u->destinations[u->historyCount][MAX_STR-1] = '\0';
        u->historyCount++;
    }
    fclose(fp);
}

int loadRides(const char *fname, Ride **rides, int *rideCap) {
    FILE *fp = fopen(fname, "r");
    if (!fp) { printf("Could not open %s\n", fname); exit(1); }
    char line[512];
    fgets(line, sizeof(line), fp);
    int rideCount = 0;
    while (fgets(line, sizeof(line), fp)) {
        if (rideCount == *rideCap) { *rideCap *= 2; *rides = realloc(*rides, (*rideCap) * sizeof(Ride)); }
        char *fields[8] = {0}; int col = 0;
        char temp[512]; strncpy(temp, line, sizeof(temp));
        char *token = strtok(temp, ","); while (token && col < 8) fields[col++] = token, token = strtok(NULL, ",");
        if (col < 6 || !fields[2] || !fields[4] || !fields[5] || !fields[6]) continue;
        Ride r;
        strncpy(r.name, fields[2], MAX_STR-1); cleanField(r.name);
        formatDate(fields[4], r.travelDate); // CSV to DD-MM-YYYY
        char place[MAX_STR]; strncpy(place, fields[5], MAX_STR-1); cleanField(place);
        splitPlaceFix(place, r.source, r.destination);
        strncpy(r.departTime, fields[6], MAX_STR-1); cleanField(r.departTime);
        (*rides)[rideCount++] = r;
    }
    fclose(fp);
    return rideCount;
}

void printRides(Ride *rides, int rideCount) {
    for (int i = 0; i < rideCount; i++) {
        Ride *r = &rides[i];
        printf("%d. Name: %s | Date: %s | From: %s | To: %s | Departs: %s\n",
            i+1, r->name, r->travelDate, r->source, r->destination, r->departTime);
    }
}

void recommendRide(Ride *rides, int rideCount) {
    char direction[16], loc[MAX_STR], inputDate[MAX_STR], formattedDate[MAX_STR];
    printf("Are you GOING from VITC or RETURNING to VITC? (type 'going' or 'returning'): ");
    scanf("%15s", direction);

    printf("Enter travel date (DD-MM-YYYY): ");
    scanf("%s", inputDate);
    formatDate(inputDate, formattedDate); // This just standardizes (pass-through for DD-MM-YYYY input)

    int found = 0;
    if (strcmp(direction, "going") == 0) {
        printf("Enter destination location: ");
        scanf(" %[^\n]", loc);
        for (int i = 0; i < rideCount; i++) {
            if (strcasecmp(rides[i].source, "VITC") == 0 &&
                strstr(rides[i].destination, loc) &&
                strcmp(rides[i].travelDate, formattedDate) == 0) {
                found = 1;
                printf("Name: %s | From: %s | To: %s | Date: %s | Departs: %s\n",
                    rides[i].name, rides[i].source, rides[i].destination, rides[i].travelDate, rides[i].departTime);
            }
        }
        if (!found)
            printf("No rides found going from VITC to %s on %s.\n", loc, formattedDate);
    } else if (strcmp(direction, "returning") == 0) {
        printf("Enter your source location (from): ");
        scanf(" %[^\n]", loc);
        for (int i = 0; i < rideCount; i++) {
            if (strcasecmp(rides[i].destination, "VITC") == 0 &&
                strstr(rides[i].source, loc) &&
                strcmp(rides[i].travelDate, formattedDate) == 0) {
                found = 1;
                printf("Name: %s | From: %s | To: %s | Date: %s | Departs: %s\n",
                    rides[i].name, rides[i].source, rides[i].destination, rides[i].travelDate, rides[i].departTime);
            }
        }
        if (!found)
            printf("No rides found returning to VITC from %s on %s.\n", loc, formattedDate);
    } else {
        printf("Please enter either 'going' or 'returning'.\n");
    }
}

void printUserHistory(UserHistory *users, int userCount) {
    for (int i = 0; i < userCount; i++) {
        UserHistory *u = &users[i];
        printf("User: %s | Past Destinations: ", u->name);
        for (int j = 0; j < u->historyCount; j++) {
            printf("%s", u->destinations[j]);
            if (j < u->historyCount - 1) printf(", ");
        }
        printf("\n");
    }
}

void freeHistories(UserHistory *users, int userCount) {
    for (int i = 0; i < userCount; i++) {
        for (int j = 0; j < users[i].historyCount; j++)
            free(users[i].destinations[j]);
        free(users[i].destinations);
    }
}

int main() {
    int userCap = 100, rideCap = 100;
    UserHistory *users = malloc(userCap * sizeof(UserHistory));
    int userCount = 0;
    Ride *rides = malloc(rideCap * sizeof(Ride));
    int rideCount = 0;
    loadRideHistory("rides.csv", &users, &userCount, &userCap);
    rideCount = loadRides("rides_current.csv", &rides, &rideCap);

    while (1) {
        printf("\nMenu:\n1. Show All Available Rides\n2. Find Ride\n3. Show User History\n4. Exit\nChoice: ");
        int choice; scanf("%d", &choice); getchar();
        switch (choice) {
            case 1: printRides(rides, rideCount); break;
            case 2: recommendRide(rides, rideCount); break;
            case 3: printUserHistory(users, userCount); break;
            case 4: freeHistories(users, userCount); free(users); free(rides); printf("Exiting.\n"); return 0;
            default: printf("Invalid choice.\n");
        }
    }
}
