#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <sys/socket.h> // for socket(), connect(), send(), and recv() 
#include <arpa/inet.h>  // for sockaddr_in and inet_addr() 
#include <string.h>     // for memset() 

#define RCVBUFSIZE 40   // Size of receive buffer
#define FLOWERS 40
#define TIME_SLEEP 2

// тип запроса или ответа
typedef enum {
    NEW_DAY, WATER
} MessageStatus;

void DieWithError(char *errorMessage);  // Error handling function

int main(int argc, char *argv[]) {
    srand(time(NULL));

    int sock;                        // Socket descriptor 
    struct sockaddr_in server_address; // Echo server address 
    unsigned short port;     // Echo server port 
    char *servIP;                    // Server IP address (dotted quad)

    if (argc != 3)  // Test for correct number of arguments
    {
        fprintf(stderr, "Usage: %s <Server IP> <Server Port>\n",
                argv[0]);
        exit(1);
    }

    servIP = argv[1];             // First arg: server IP address (dotted quad)
    port = atoi(argv[2]); // Use given port, if any

    sleep(1);
    // Create a reliable, stream socket using TCP
    if ((sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
        DieWithError("socket() failed");
    }

    // Construct the server address structure
    memset(&server_address, 0, sizeof(server_address));     // Zero out structure
    server_address.sin_family = AF_INET;             // Internet address family
    server_address.sin_addr.s_addr = inet_addr(servIP);   // Server IP address
    server_address.sin_port = htons(port); // Server port

    // Establish the connection to the echo server
    if (connect(sock, (struct sockaddr *) &server_address, sizeof(server_address)) < 0) {
        DieWithError("connect() failed\n");
    }

    char response[RCVBUFSIZE];
    char *request = (char *) malloc(41 * sizeof(char));
    for (int i = 0; i < 40; i++) {
       request[i] = 'a'; // i-й цыеток не нуждается  в поливе
    }
    request[40] = '\0';
    int day = 1;
    char flag = 0;
    for (;;) {
        send(sock, request, 1, 0);
        response[0] = 0;
        recv(sock, response, 42, 0);
        if (response[0] == 'n') {
            if (flag == 1) {
                flag = 0;
                printf("flag");
                continue;
            }
            flag = 1;
            if (day == 16) {
                break;
            }
            printf("=============================================================\n");
            printf("Начинается новый день %d!\n", day);
            printf("Увядшие цыеты на клумбе: ");
            int count_of_NEED_WATER = 0;
            for (int i = 1; i < 41; ++i) {
                if (response[i] == '1') {
                    count_of_NEED_WATER++;
                    printf("%d, ", i); // выводим номера увядших цветов
                }
            }
            printf("\n");
            printf("Свежих цветов: %d, нуждаются в поливе : %d.\n", 40 - count_of_NEED_WATER, count_of_NEED_WATER);
            day++;
        } else if (response[0] == 'a' || response[0] == 'b') {
            for (int i = 0; i < 40; ++i) {
                if (response[1 + i] == '1') {
                    if (response[0] == 'a') {
                        printf("Садовник Alice поливает цветок с номером %d \n", i + 1);
                    } else {
                        printf("Садовник Bob поливает цветок с номером %d \n", i + 1);
                    }
                }
            }
        }
    }
    close(sock);
    exit(0);
}