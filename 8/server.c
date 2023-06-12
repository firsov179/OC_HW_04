#include <stdio.h>      /* for printf() and fprintf() */
#include <sys/socket.h> /* for socket(), bind(), and connect() */
#include <arpa/inet.h>  /* for sockaddr_in and inet_ntoa() */
#include <stdlib.h>     /* for atoi() and exit() */
#include <string.h>     /* for memset() */
#include <unistd.h>     /* for close() */
#include <pthread.h>
#include <semaphore.h>

#define MAXPENDING 5    /* Maximum outstanding connection requests */
#define RCVBUFSIZE 255   /* Size of receive buffer */

char cur_flowerbed[41];
char cur_water[41];
sem_t *semaphore;
int flag = 1;

void DieWithError(char *errorMessage);  /* Error handling function */


void FlowerbedHandle(char* request, char* response) {
    sem_wait(semaphore);
    for (int i = 0; i < 40; ++i) {
        cur_flowerbed[i] = request[i + 1];
    }
    response[0] = 'f';
    for (int i = 0; i < 40; ++i) {
        response[i + 1] = cur_water[i];
    }
    sem_post(semaphore);
}

void GardenerHandle(char* request, char* response) {
    sem_wait(semaphore);
    if (request[0] == 'N') {
        // начало дня у садовника -> передаем информацию о цветах которые нужно полить.
        response[0] = 'n';
        for (int i = 0; i < 41; ++i) {
            response[i + 1] = cur_flowerbed[i];
        }
        if (flag == 1) {
            for (int i = 0; i < 40; ++i) {
                cur_water[i] = '0';
            }
            flag = 2;
        } else {
            flag = 1;
        }
    } else {
        /* Send received string and receive again until end of transmission */
        response[0] = 'g';
        for (int i = 0; i < 40; ++i) {
            if (request[i + 1] == '1' && cur_water[i] == '0') {
                cur_water[i] = '1';
                response[i + 1] = '1';
            } else {
                response[i + 1] = '0';
            }
        }
    }
    sem_post(semaphore);
}

#define BUFFER_SIZE 1024
#define MAX_PRINTER 10

struct sockaddr_in printers[MAX_PRINTER];
int max_printer = 0;

void printer(int server_fd, const char* message) {
    for (int i = 0; i < max_printer; ++i) {
        sendto(server_fd, message, strlen(message), 0, (struct sockaddr *)&printers[i], sizeof(printers[i]));
    }
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Usage: %s <host> <port>\n", argv[0]);
        return 1;
    }
    int day = 0;
    const char* host = argv[1];
    int port = atoi(argv[2]);
    int server_fd;
    struct sockaddr_in address, client_addr;
    socklen_t client_addr_len = sizeof(client_addr);
    char buffer[BUFFER_SIZE];

    // Create socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_DGRAM, 0)) == 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

    // Bind the socket to the specified address and port
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }
    sem_t local_semaphore;
    semaphore = &local_semaphore;
    sem_init(semaphore, 0, 1);
    printf("Server listening on %s:%d\n", host, port);
    for (int i = 0; i < 40; ++i) {
        cur_water[i] = '0';
        cur_flowerbed[i] = '0';
    }
    // Handle incoming requests
    for(;;) {
        memset(buffer, 0, BUFFER_SIZE);
        // Receive the message from the client
        recvfrom(server_fd, buffer, BUFFER_SIZE, 0, (struct sockaddr *)&client_addr, &client_addr_len);
        char response[42];
        if (buffer[0] == 'P') {
            if (max_printer < MAX_PRINTER) {
                printers[max_printer++] = client_addr;
                printf("Add printer\n");
            }
            continue;
        } else if (buffer[0] == 'F') {
            FlowerbedHandle(&buffer, &response);
        } else if (buffer[0] == 'N' || buffer[0] == 'A'|| buffer[0] == 'B') {
            GardenerHandle(&buffer, &response);
            if (day == 31 * 2) {
                return 0;
            }
            if (buffer[0] == 'N') {
                day++;
            }
        }
        if (response[0] == 'g') {
            if (buffer[0] == 'A') {
                response[0] = 'a';
            } else {
                response[0] = 'b';
            }
        }
        sendto(server_fd, response, strlen(response), 0, (struct sockaddr *)&client_addr, client_addr_len);
        printer(server_fd, response);
    }
    close(server_fd);
    return 0;
}
