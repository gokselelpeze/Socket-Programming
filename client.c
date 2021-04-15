#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<unistd.h>
#include<pthread.h>
#include <stdbool.h>

#define MESSAGE_LENGTH  200
bool exitFlag = false;
int sockfd = 0;

bool startsWith(const char *a, const char *b) {
    if (strncmp(a, b, strlen(b)) == 0)
        return true;
    return false;
}

void messageReceiver() {
    char msg[MESSAGE_LENGTH];
    while (1) {
        int receive = recv(sockfd, msg, MESSAGE_LENGTH, 0);
        if (receive > 0) {
            if (startsWith(msg, "-exit")) {
                exitFlag = true;
                break;
            }
            printf("%s", msg);
        } else if (receive == 0) {
            break;
        }
        memset(msg, '\0', sizeof(msg));
    }
}
void delSpace(char *str, int length) {
    for (int i = 0; i < length; i++) {
        if (str[i] == '\n') {
            str[i] = '\0';
            break;
        }
    }
}

_Noreturn void messageSender() {
    char message[MESSAGE_LENGTH];
    char from[] = "from: phone";
    char msg[] = "message: msg";
    char *templateJSON ="from:";
    char delim[] = " ";
    while (true) {
        fgets(message, MESSAGE_LENGTH, stdin);
        delSpace(message, sizeof(message));
        send(sockfd, message, strlen(message), 0);
        memset(message, 0, sizeof(message));
    }
}

int main() {
    struct sockaddr_in serverAdr;
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    serverAdr.sin_family = AF_INET;
    serverAdr.sin_addr.s_addr = INADDR_ANY;
    serverAdr.sin_port = htons(3205);
    inet_pton(AF_INET, "127.0.0.1", &serverAdr.sin_addr);

    int connection = connect(sockfd, (struct sockaddr *) &serverAdr, sizeof(serverAdr));
    if (connection != -1) {
        printf("Connected Successfully\n");
    } else {
        printf("Connection Failed\n");
    }
    pthread_t senderThread;
    pthread_t receiverThread;

    if (pthread_create(&senderThread, NULL, (void *) messageSender, NULL) != 0) {
        printf("messageSender Thread Error");
    }

    if (pthread_create(&receiverThread, NULL, (void *) messageReceiver, NULL) != 0) {
        printf("messageReceiver Thread Error");
    }
    printf("Welcome To DEU Signal v1.0 \n");

    while (true) {
        if (exitFlag) {
            break;
        }
    }
    printf("You left from program...\n");
    close(sockfd);
    return true;
}