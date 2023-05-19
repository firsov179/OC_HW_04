#include <stdio.h>  /* for perror() */
#include <stdlib.h> /* for exit() */
#include <string.h>

// тип запроса или ответа
typedef enum {NEW_DAY, WATER, SUCCESS, FAILURE} MessageStatus;

void DieWithError(char *errorMessage) {
    perror(errorMessage);
    exit(1);
}

char * CreateRequest(char *request, MessageStatus *status) {
    char* data = (char*)malloc((strlen(request) + 1) * sizeof(char));
    if (*status == NEW_DAY) {
        data[0] = 'n';
    } else if (*status == WATER) {
        data[0] = 'w';
    }
    return data;
}

char * CreateResponse(char *request, MessageStatus *status) {
    char* data = (char*)malloc((strlen(request) + 1) * sizeof(char));
    if (*status == SUCCESS) {
        data[0] = 's';
    } else if (*status == FAILURE) {
        data[0] = 'f';
    }
    return data;
}

char * Parse(char *request, int size, MessageStatus *status) {
    if (request[0] == 'n') {
        *status = NEW_DAY;
    } else if (request[0] == 'w') {
        *status = WATER;
    } if (request[0] == 's') {
        *status = SUCCESS;
    } if (request[0] == 'f') {
        *status = FAILURE;
    }
    char* data = (char*)malloc((size - 1) * sizeof(char));
    for (int i = 1; i < size; ++i) {
        data[i - 1] = request[i];
    }
    return data;
}

