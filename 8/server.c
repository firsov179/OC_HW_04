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
        int flowerbed_socket = ((int *)arg)[0];
        int printer_socket[11];
        for (int i = 0; i < 11; ++i) {
            printer_socket[i] = ((int *)arg)[i + 2];
        }
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
        sem_post(semaphore);
    }
}

void *GardenerHandle(void *arg) {
    int flag = 1;
    for (;;) {
        int gardener_socket = ((int *)arg)[0];
        int id = ((int *)arg)[1];
        int printer_socket[11];
        for (int i = 0; i < 11; ++i) {
            printer_socket[i] = ((int *)arg)[i + 2];
        }
        sem_wait(semaphore);
        char request[RCVBUFSIZE];

        /* Receive message from client */
        if ((recv(gardener_socket, request, RCVBUFSIZE, 0)) < 0)
            DieWithError("recv() failed");

        if (request[0] == 'n') {
            // начало дня у садовника -> передаем информацию о цветах которые нужно полить.
            send(gardener_socket, cur_flowerbed, 41, 0);
            char response[42];
            response[0] = 'n';
            for (int i = 0; i < 41; ++i) {
                response[i + 1] = cur_flowerbed[i];
            }
            for (int i = 0; printer_socket[i] != -1; ++i) {
                send(printer_socket[i], response, 42, 0);
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
            send(gardener_socket, response, 41, 0);
            char response_print[42];
            if (id == 1) {
                response_print[0] = 'a';
            } else {
                response_print[0] = 'b';
            }
            for (int i = 0; i < 41; ++i) {
                response_print[i + 1] = response[i];
            }
            for (int i = 0; printer_socket[i] != -1; ++i) {
                send(printer_socket[i], response_print, 42, 0);
            }
           
        }
        sem_post(semaphore);
    }
}

int main(int argc, char *argv[]) {
    int servSock;                    /* Socket descriptor for server */
    int flowerbed_sock, gardener_socket, gardener_sock2, printer_sock[11];                    /* Socket descriptor for client */
    struct sockaddr_in servAddr; /* Local address */
    struct sockaddr_in flowerbed_addr, gardener_addr_1, gardener_addr_2; /* Client address */
    struct sockaddr_in printer_addr[11];
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
    if (argc != 3)     /* Test for correct number of arguments */
    {
        fprintf(stderr, "Usage:  %s <Server Port> <Count of printer>\n", argv[0]);
        exit(1);
    }

    port = atoi(argv[1]);  /* First arg:  local port */
    int n = atoi(argv[2]);  /* First arg:  local port */
    if (n > 10) {
        DieWithError("A lot of printers");
    }

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

    if ((gardener_socket = accept(servSock, (struct sockaddr *) &gardener_addr_1,
                                 &clntLen)) < 0)
        DieWithError("accept() failed");
    printf("Handling client %s\n", inet_ntoa(gardener_addr_1.sin_addr));

    if ((gardener_sock2 = accept(servSock, (struct sockaddr *) &gardener_addr_2,
                                 &clntLen)) < 0)
        DieWithError("accept() failed");
    printf("Handling client %s\n", inet_ntoa(gardener_addr_2.sin_addr));
    for (int i = 0; i < n; ++i) {
        if ((printer_sock[i] = accept(servSock, (struct sockaddr *) &(printer_addr[i]),
                                 &clntLen)) < 0)
            DieWithError("accept() failed");
        printf("Handling client %s\n", inet_ntoa(printer_addr[i].sin_addr));
    }
    printer_sock[n] = -1;

    pthread_t thread1, thread2, thread3;
    int data1[13];
    data1[0] = gardener_socket;
    data1[1] = 1;
    for (int i = 0; i < 11; ++i) {
        data1[2 + i] = printer_sock[i];
    }
    int result1 = pthread_create(&thread1, NULL,
                                 GardenerHandle, (void *) &data1);
    int data2[13];
    data2[0] = gardener_sock2;
    data2[1] = 2;
    for (int i = 0; i < 11; ++i) {
        data2[2 + i] = printer_sock[i];
    }
    int result2 = pthread_create(&thread2, NULL,
                                 GardenerHandle, (void *) &data2);
    int data3[13];
    data3[0] = flowerbed_sock;
    for (int i = 0; i < 11; ++i) {
        data3[2 + i] = printer_sock[i];
    }
    printf("%d", flowerbed_sock);
    int result3 = pthread_create(&thread3, NULL,
                                 FlowerbedHandle, (void *) &data3);
    sleep(100);
    if (result1 || result2 || result3) {
        printf("The threads could not be joined.\n");
        exit(2);
    }
}