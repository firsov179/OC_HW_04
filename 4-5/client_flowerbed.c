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
    char *res = (char *) malloc(42 * sizeof(char));
    for (int i = 0; i < 40; i++) {
        if (flowerbed.flowers[i] == NORMAL) { // создаем строку с информацией о цветах.
            res[i + 1] = '0'; // i-й цыеток не нуждается в поливе
        } else {
            res[i + 1] = '1'; // i-й цыеток нуждается в поливе
        }
    }
    res[0] = 'F';
    res[41] = '\0';
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
    if (argc != 3)  // Test for correct number of arguments
    {
        fprintf(stderr, "Usage: %s <Server IP> <Server Port>\n", argv[0]);
        exit(1);
    }
    for (int i = 0; i < FLOWERS; i++) {
        flowerbed.flowers[i] = NORMAL;
    }
    const char* server_ip = argv[1];
    int server_port = atoi(argv[2]);

    int sock = 0, valread;
    struct sockaddr_in serv_addr;
    socklen_t addr_len = sizeof(serv_addr);
    char buffer[42] = {0};

    // Create socket file descriptor
    if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(server_port);

    // Convert IPv4 and IPv6 addresses from text to binary form
    if (inet_pton(AF_INET, server_ip, &serv_addr.sin_addr) <= 0) {
        perror("Invalid address/Address not supported");
        exit(EXIT_FAILURE);
    }
    time_t last, cur;
    last = time(NULL);
    flowerbed.day = 0;
    char response[42];
    for (;;) {
        cur = time(NULL);
        if (last + TIME_SLEEP <= cur) {
            printf("Завядшие цветы вечером:");
            printFlowerbed();
            if (flowerbed.day == 31) {
                return 0;
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
        sendto(sock, request, strlen(request), 0, (struct sockaddr *)&serv_addr, addr_len);
        if (recvfrom(sock, response, 42, 0, (struct sockaddr *)&serv_addr, &addr_len) <= 0) {
            DieWithError("recv() failed or connection closed prematurely\n");
        }
        water(response);
    }
    // Close the socket
    close(sock);

    return 0;
}