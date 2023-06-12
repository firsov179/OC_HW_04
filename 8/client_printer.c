#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <sys/socket.h> // for socket(), connect(), send(), and recv() 
#include <arpa/inet.h>  // for sockaddr_in and inet_addr() 
#include <string.h>     // for memset() 

#define RCVBUFSIZE 40   // Size of receive request
#define FLOWERS 40
#define TIME_SLEEP 2

// тип запроса или ответа
typedef enum {
    NEW_DAY, WATER
} MessageStatus;

void DieWithError(char *errorMessage);  // Error handling function

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define BUFFER_SIZE 1024

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Port num required");
        return 1;
    }

    const char* server_ip = argv[1];
    int server_port = atoi(argv[2]);

    int sock = 0;
    struct sockaddr_in serv_addr;
    socklen_t addr_len = sizeof(serv_addr);
    char request[BUFFER_SIZE] = {0};
    int day = 1;
    // Create socket file descriptor
    if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }
    int flag = 1;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(server_port);

    // Convert IPv4 and IPv6 addresses from text to binary form
    if (inet_pton(AF_INET, server_ip, &serv_addr.sin_addr) <= 0) {
        perror("Invalid address/Address not supported");
        exit(EXIT_FAILURE);
    }

    printf("Viewer started\n");
    // Register new printer
    sendto(sock, "P", strlen("P"), 0, (struct sockaddr *)&serv_addr, addr_len);
    while(1) {
        // Receive the current message from the server
        memset(request, 0, BUFFER_SIZE);
        recvfrom(sock, request, BUFFER_SIZE, 0, (struct sockaddr *)&serv_addr, &addr_len);
        if (request[0] == 'n') {
            if (flag == 1) {
                flag = 0;
                continue;
            }
            flag = 1;
            printf("=============================================================\n");
            printf("Начинается новый день %d!\n", day);
            printf("Увядшие цыеты на клумбе: ");
            int count_of_NEED_WATER = 0;
            for (int i = 1; i < 41; ++i) {
                if (request[i] == '1') {
                    count_of_NEED_WATER++;
                    printf("%d, ", i); // выводим номера увядших цветов
                }
            }
            printf("\n");
            printf("Свежих цветов: %d, нуждаются в поливе : %d.\n", 40 - count_of_NEED_WATER, count_of_NEED_WATER);
            day++;
            if (day == 31) {
                return 0;
            }
        } else if (request[0] == 'a' || request[0] == 'b') {
            for (int i = 0; i < 40; ++i) {
                if (request[1 + i] == '1') {
                    if (request[0] == 'a') {
                        printf("Садовник Alice поливает цветок с номером %d \n", i + 1);
                    } else {
                        printf("Садовник Bob поливает цветок с номером %d \n", i + 1);
                    }
                }
            }
        }
    }

    // Close the socket
    close(sock);
    return 0;
}