#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <sys/socket.h> // for socket(), connect(), send(), and recv() 
#include <arpa/inet.h>  // for sockaddr_in and inet_addr() 
#include <string.h>     // for memset() 

#define RCVBUFSIZE 255   // Size of receive buffer 
#define FLOWERS 40
#define TIME_SLEEP 2

// Состояние цветка
typedef enum {
    NORMAL, NEED_WATER
} Status;
// тип запроса или ответа
typedef enum {
    NEW_DAY, WATER
} MessageStatus;

// Клумба
typedef struct {
    Status flowers[FLOWERS];
    int day;
} Flowerbed;

Flowerbed flowerbed;

void new_day() {
    int n = 10;
    for (int i = 0; i < 40; i++) {
        if (flowerbed.flowers[i] == NORMAL) {
            if (rand() % 2 == 0) {
                flowerbed.flowers[i] = NEED_WATER;
                n--;
            }
            if (n == 0) {
                break;
            }
        }
    }
    flowerbed.day++;
}

char *requestBuilder() {
    char *res = (char *) malloc(41 * sizeof(char));
    for (int i = 0; i < 40; i++) {
        if (flowerbed.flowers[i] == NORMAL) { // создаем строку с информацией о цветах.
            res[i] = '0'; // i-й цыеток не нуждается  в поливе
        } else {
            res[i] = '1'; // i-й цыеток нуждается  в поливе
        }
    }
    res[40] = '\0';
    return res;
}

void water(char *request) {
   for (int i = 0; i < FLOWERS; ++i) {
       if (request[i] == '1' && flowerbed.flowers[i] == NEED_WATER) {
           flowerbed.flowers[i] = NORMAL;
           printf("Кто-то полил цветок с номером: %d\n", i);
       }
   }
}

void printFlowerbed() { // вывод информации о клумбе
    int count_of_NORMAL = 0;
    int count_of_NEED_WATER = 0;
    for (int i = 0; i < FLOWERS; i++) {
        if (flowerbed.flowers[i] == NORMAL) {
            count_of_NORMAL++;
        } else {
            count_of_NEED_WATER++;
            printf("%d, ", i); // выводим номера увядших цветов
        }
    }
    printf("\n");
    printf("Свежих цветов: %d, нуждаются в поливе : %d.\n", count_of_NORMAL, count_of_NEED_WATER);
}

void DieWithError(char *errorMessage);  // Error handling function 
char *Parse(char *request, int size, MessageStatus *status);

int main(int argc, char *argv[]) {
    srand(time(NULL));
    for (int i = 0; i < FLOWERS; i++) {
        flowerbed.flowers[i] = NORMAL;
    }
    int sock;                        // Socket descriptor 
    struct sockaddr_in server_address; // Echo server address 
    unsigned short port;     // Echo server port
    char *servIP;                    // Server IP address (dotted quad) 

    char response[RCVBUFSIZE];     // Buffer for echo string 
    unsigned int requestSize;      // Length of string to echo 
    int bytesRcvd, totalBytesRcvd;   // Bytes read in single recv() and total bytes read 

    if (argc != 3)  // Test for correct number of arguments 
    {
        fprintf(stderr, "Usage: %s <Server IP> <Server Port>\n",
                argv[0]);
        exit(1);
    }

    servIP = argv[1];             // First arg: server IP address (dotted quad)
    port = atoi(argv[2]); // Use given port, if any


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
    time_t last, cur;
    last = time(NULL);
    flowerbed.day = 0;
    for (;;) {
        cur = time(NULL);
        if (last + TIME_SLEEP <= cur) {
            printf("Завядшие цветы вечером:");
            printFlowerbed();
            if (flowerbed.day == 16) {
                break;
            }
            last = cur; // Начинаем новый день каждую TIME_SLEEP секунду
            new_day();
            printf("=============================================================\n");
            printf("У лужайки начинается новый день %d!\n", flowerbed.day);
            printf("Увядают цветы c номерами: ");
            printFlowerbed();
        }
        char *request = requestBuilder();

        // Send the string to the server
        send(sock, request, 41, 0);
        if ((bytesRcvd = recv(sock, response, 41, 0)) <= 0) {
            DieWithError("recv() failed or connection closed prematurely\n");
        }
        water(response);
    }
    close(sock);
    exit(0);
}