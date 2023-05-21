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
    char *res = (char *) malloc(41 * sizeof(char));
    for (int i = 0; i < 40; i++) {
        if (water_cou > 0 && request[i] == '1') { // создаем строку с информацией о цветах.
            res[i] = '1'; // i-й цыеток не нуждается  в поливе
            water_cou--;
        } else {
            res[i] = '0'; // i-й цыеток нуждается  в поливе
        }
    }
    res[40] = '\0';
    return res;
}

void DieWithError(char *errorMessage);  // Error handling function

int main(int argc, char *argv[]) {
    srand(time(NULL));

    int sock;                        // Socket descriptor 
    struct sockaddr_in server_address; // Echo server address 
    unsigned short port;     // Echo server port 
    char *servIP;                    // Server IP address (dotted quad) 

    char response[RCVBUFSIZE];     // Buffer for echo string 
    unsigned int requestSize;      // Length of string to echo 
    int bytesRcvd, totalBytesRcvd;   // Bytes read in single recv() and total bytes read 

    if (argc != 5)  // Test for correct number of arguments
    {
        fprintf(stderr, "Usage: %s <Server IP> <Server Port> <Name> <Count water>\n",
                argv[0]);
        exit(1);
    }

    servIP = argv[1];             // First arg: server IP address (dotted quad)
    port = atoi(argv[2]); // Use given port, if any
    name = argv[3];
    max_water = atoi(argv[4]);

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
    for (;;) {
        cur = time(NULL);
        if (last + TIME_SLEEP <= cur) {
            last = cur; // Начинаем новый день каждую TIME_SLEEP секунду
            if (day == 16) {
                break;
            }
            day++;
            printf("=============================================================\n");
            printf("У садовника %s начинается новый день %d!\n", name, day);
            real_water = max_water;
            char *c = (char *) malloc(41 * sizeof(char));
            c[0] = 'n';
            c[1] = '\0';
            send(sock, c, 41, 0);
            if ((bytesRcvd = recv(sock, response, 41, 0)) <= 0) {
                DieWithError("recv() failed or connection closed prematurely\n");
            }
            for (int i = 0; i < 40; ++i) {
                need_water[i] = response[i];
            }
        }
        char *request = water(need_water);
        // Send the string to the server
        send(sock, request, 41, 0);
        if ((bytesRcvd = recv(sock, response, 41, 0)) <= 0) {
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
    close(sock);
    exit(0);
}