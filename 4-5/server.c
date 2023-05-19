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

void DieWithError(char *errorMessage);  /* Error handling function */

void *FlowerbedHandle(void *arg) {
    for (;;) {
        int flowerbed_socket = *(int *) arg;
        sem_wait(semaphore);
        char request[RCVBUFSIZE];

        /* Receive message from client */
        if ((recv(flowerbed_socket, request, RCVBUFSIZE, 0)) < 0)
            DieWithError("recv() failed");

        /* Send received string and receive again until end of transmission */
        for (int i = 0; i < 40; ++i) {
            cur_flowerbed[i] = request[i];
        }
        /* Message response to client */
        send(flowerbed_socket, cur_water, 41, 0);

        for (int i = 0; i < 40; ++i) {
            cur_water[i] = '0';
        }
        sem_post(semaphore);
    }
}

void *GardenerHandle(void *arg) {
    for (;;) {
        int flowerbed_socket = *(int *) arg;
        sem_wait(semaphore);
        char request[RCVBUFSIZE];

        /* Receive message from client */
        if ((recv(flowerbed_socket, request, RCVBUFSIZE, 0)) < 0)
            DieWithError("recv() failed");

        if (request[0] == 'n') {
            // начало дня у садовника -> передаем информацию о цветах которые нужно полить.
            send(flowerbed_socket, cur_flowerbed, 41, 0);
        } else {
            /* Send received string and receive again until end of transmission */
            char response[41];
            for (int i = 0; i < 40; ++i) {
                if (request[i] == '1' && cur_water[i] == '0') {
                    cur_water[i] = '1';
                    response[i] = '1';
                } else {
                    response[i] = '0';
                }
            }
            /* Message response to client */
            send(flowerbed_socket, response, 41, 0);
        }
        sem_post(semaphore);
    }
}

int main(int argc, char *argv[]) {
    int servSock;                    /* Socket descriptor for server */
    int flowerbed_sock, gardener_sock1, gardener_sock2;                    /* Socket descriptor for client */
    struct sockaddr_in servAddr; /* Local address */
    struct sockaddr_in flowerbed_addr, gardener_addr_1, gardener_addr_2; /* Client address */
    unsigned short port;     /* Server port */
    unsigned int clntLen;            /* Length of client address data structure */
    sem_t local_semaphore;
    semaphore = &local_semaphore;
    sem_init(semaphore, 0, 1);
    printf("Semaphore created!");
    for (int i = 0; i < 40; ++i) {
        cur_water[i] = '0';
        cur_flowerbed[i] = '0';
    }
    cur_water[40] = '\0';
    cur_flowerbed[40] = '\0';
    if (argc != 2)     /* Test for correct number of arguments */
    {
        fprintf(stderr, "Usage:  %s <Server Port>\n", argv[0]);
        exit(1);
    }

    port = atoi(argv[1]);  /* First arg:  local port */

    /* Create socket for incoming connections */
    if ((servSock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
        DieWithError("socket() failed");

    /* Construct local address structure */
    memset(&servAddr, 0, sizeof(servAddr));   /* Zero out structure */
    servAddr.sin_family = AF_INET;                /* Internet address family */
    servAddr.sin_addr.s_addr = htonl(INADDR_ANY); /* Any incoming interface */
    servAddr.sin_port = htons(port);      /* Local port */

    /* Bind to the local address */
    if (bind(servSock, (struct sockaddr *) &servAddr, sizeof(servAddr)) < 0)
        DieWithError("bind() failed");

    printf("Server IP address = %s. Wait...\n", inet_ntoa(servAddr.sin_addr));

    /* Mark the socket so it will listen for incoming connections */
    if (listen(servSock, MAXPENDING) < 0)
        DieWithError("listen() failed");
    clntLen = sizeof(flowerbed_addr);


    /* Wait for a client to connect */
    if ((flowerbed_sock = accept(servSock, (struct sockaddr *) &flowerbed_addr,
                                 &clntLen)) < 0)
        DieWithError("accept() failed");
    printf("Handling client %s\n", inet_ntoa(flowerbed_addr.sin_addr));

    if ((gardener_sock1 = accept(servSock, (struct sockaddr *) &gardener_addr_1,
                                 &clntLen)) < 0)
        DieWithError("accept() failed");
    printf("Handling client %s\n", inet_ntoa(gardener_addr_1.sin_addr));

    if ((gardener_sock2 = accept(servSock, (struct sockaddr *) &gardener_addr_2,
                                 &clntLen)) < 0)
        DieWithError("accept() failed");
    printf("Handling client %s\n", inet_ntoa(gardener_addr_2.sin_addr));

    pthread_t thread1;
    pthread_t thread2;
    pthread_t thread3;
    int result1 = pthread_create(&thread1, NULL,
                                 GardenerHandle, (int *) &gardener_sock1);
    int result2 = pthread_create(&thread2, NULL,
                                 GardenerHandle, (int *) &gardener_sock2);
    int result3 = pthread_create(&thread3, NULL,
                                 FlowerbedHandle, (int *) &flowerbed_sock);

    sleep(100);
    if (result1 || result2 || result3) {
        printf("The threads could not be joined.\n");
        exit(2);
    }
}