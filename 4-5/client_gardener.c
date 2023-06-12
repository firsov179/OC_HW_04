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

// тип запроса или ответа
typedef enum {
    NEW_DAY, WATER
} MessageStatus;

int water_cou = 0;
int real_water = 0;
int max_water = 0;
char* name;
int day = 0;

char* water(char *request) {
    water_cou = real_water;
    char *res = (char *) malloc(42 * sizeof(char));
    for (int i = 0; i < 40; i++) {
        if (water_cou > 0 && request[i] == '1') { // создаем строку с информацией о цветах.
            res[i + 1] = '1'; // i-й цветок не нуждается в поливе
            water_cou--;
        } else {
            res[i + 1] = '0'; // i-й цветок нуждается в поливе
        }
    }
    res[41] = '\0';
    return res;
}

void DieWithError(char *errorMessage);  // Error handling function

int main(int argc, char *argv[]) {
    srand(time(NULL));
    if (argc != 5)  // Test for correct number of arguments
    {
        fprintf(stderr, "Usage: %s <Server IP> <Server Port> <Name> <Count water>\n", argv[0]);
        exit(1);
    }
    const char* server_ip = argv[1];
    int server_port = atoi(argv[2]);
    name = argv[3];
    max_water = atoi(argv[4]);

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
    time_t last, cur, start;
    last = time(NULL);
    start = last;
    char *need_water = (char *) malloc(41 * sizeof(char));
    for (int i = 0; i < 40; ++i) {
        need_water[i] = '0';
    }
    need_water[40] = '\0';
    day = 0;
    printf("У садовника %s начинается новый день %d!\n", name, day);
    char response[42];
    for (;;) {
        cur = time(NULL);
        cur = time(NULL);
        if (last + TIME_SLEEP <= cur) {
            last = cur; // Начинаем новый день каждую TIME_SLEEP секунду
            day++;
            if (day == 31) {
                return 0;
            }
            printf("=============================================================\n");
            printf("У садовника %s начинается новый день %d!\n", name, day);
            real_water = max_water;
            char *c = (char *) malloc(42 * sizeof(char));
            c[0] = 'N';
            c[1] = '\0';
            sendto(sock, c, strlen(c), 0, (struct sockaddr *)&serv_addr, addr_len);
            if (recvfrom(sock, response, 42, 0, (struct sockaddr *)&serv_addr, &addr_len) <= 0) {
                DieWithError("recv() failed or connection closed prematurely\n");
            }
            for (int i = 0; i < 40; ++i) {
                need_water[i] = response[i + 1];
            }
        } else {
            char *request = water(need_water);
            request[0] = name[0] + 'A' - 'a';
            // Send the string to the server
            sendto(sock, request, strlen(request), 0, (struct sockaddr *)&serv_addr, addr_len);
            if (recvfrom(sock, response, 42, 0, (struct sockaddr *)&serv_addr, &addr_len) <= 0) {
                DieWithError("recv() failed or connection closed prematurely\n");
            }
            for (int i = 0; i < 40; ++i) {
                if (response[i] == '1') {
                    need_water[i] = '0';
                    printf("Садовник %s поливает цветок с номером %d \n", name, i);
                    real_water--;
                }
            }
        }

    }

    return 0;
}