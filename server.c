#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<unistd.h>
#include<pthread.h>
#include <stdbool.h>
#include <ctype.h>

#define MAX_GROUP 20
#define MAX_USER 100
#define MESSAGE_LENGTH 200
#define NAME_LENGTH 20
#define PASS_LENGTH 10
#define GROUP_CAPACITY 3
#define FULL 3
#define EMPTY 0
#define CLOSED 0
#define OPENED 1

// USER
typedef struct {
    struct sockaddr_in address;
    int sockfd;
    int userID;
    bool inGroup;
    int groupID;
    char username[20]; // Phone number
    char attachedPM[20]; // private message phone number
    bool isAttached; // attach control for pm
} userStruct;

// GROUP
typedef struct {
    int groupID;
    int usersInGroup[GROUP_CAPACITY];
    char groupPassword[PASS_LENGTH];
    char groupName[NAME_LENGTH];
    int groupStatus;
    int userCount;
} groupStruct;
groupStruct *groups[MAX_GROUP];
userStruct *users[MAX_USER];
int userCount = 0;
char delim[] = " ";

//Send message
void sendMessage(char *message, int id) {
    write(users[id]->sockfd, message, strlen(message));
}

//Add Client in array
void addUser(void *arg) {
    for (int i = 0; i < MAX_USER; i++) {
        userStruct *user = (userStruct *) arg;
        if (!users[i]) {
            user->userID = i;
            users[i] = user;
            user->isAttached = false;
            puts(user->username);
            break;
        }
    }
}

void closeGroup(int groupID) {
    groups[groupID]->groupStatus = CLOSED;
    memset(groups[groupID]->groupName, '\0', sizeof(groups[groupID]->groupName));
}

void sendMessageToGroup(char *message, int groupID, int id) {
    for (int i = 0; i < GROUP_CAPACITY; i++) {
        if (groups[groupID]->usersInGroup[i] != -1 && groups[groupID]->usersInGroup[i] != id) {
            sendMessage(message, groups[groupID]->usersInGroup[i]);
            write(users[id]->sockfd, message, strlen(message));
            sendMessage("\n", groups[groupID]->usersInGroup[i]);
        }
    }
}

void addUserToGroup(int groupID, void *arg) {
    userStruct *user = (userStruct *) arg;
    char message[MESSAGE_LENGTH];
    char jsonFormatted[MESSAGE_LENGTH + 20];
    if (groups[groupID]->groupStatus == OPENED && groups[groupID]->userCount < FULL) {
        for (int i = 0; i < GROUP_CAPACITY; i++) {
            if (groups[groupID]->usersInGroup[i] == -1) {
                groups[groupID]->usersInGroup[i] = user->userID;
                groups[groupID]->userCount++;
                user->inGroup = true;
                user->groupID = groupID;
                sprintf(message, "%s: You Joined to Group: %s\n", user->username, groups[groupID]->groupName);
                sendMessage(message, user->userID);
                strcat(jsonFormatted, user->username);
                strcat(jsonFormatted, ": Has been joined \n");
                sendMessageToGroup(jsonFormatted, groupID, user->userID);
                memset(jsonFormatted, '\0', sizeof(jsonFormatted));
                break;
            }
        }
    } else {
        sendMessage("Group is full\n", user->userID);
    }
}

void quitGroup(void *arg) {
    userStruct *user = (userStruct *) arg;
    for (int i = 0; i < GROUP_CAPACITY; i++) {
        if (groups[user->groupID]->usersInGroup[i] == user->userID) {
            groups[user->groupID]->usersInGroup[i] = -1;
            groups[user->groupID]->userCount--;
        }
    }
    user->inGroup = false;
    if (groups[user->groupID]->userCount == EMPTY) {
        closeGroup(user->groupID);
    }
    user->groupID = 0;
}

int findEmptyGroup() {
    int emptyGroup = -1;
    for (int i = 0; i < MAX_GROUP; i++) {
        if (groups[i]->groupStatus == CLOSED) {
            emptyGroup = i;
        }
    }
    return emptyGroup;
}

bool startsWith(const char *a, const char *b) {
    if (strncmp(a, b, strlen(b)) == 0)
        return true;
    return false;
}

int getGroupID(char *groupName) {
    int groupID = -1;
    for (int i = 0; i < MAX_GROUP; i++) {
        if (strcmp(groupName, groups[i]->groupName) == 0){}
            groupID = i;
    }
    return groupID;
}

int getUserID(char *username) {
    int userID = -1;
    for (int i = 0; i < userCount; i++) {
        puts(users[i]->username);
        if (strcmp(username, users[i]->username) == 0){
            userID = i;
            break;
        }
    }
    return userID;
}

_Noreturn void *connection(void *arg) {
    char *token;
    char *systemMessage;
    char message[MESSAGE_LENGTH];
    char jsonFormatted[MESSAGE_LENGTH + NAME_LENGTH];
    char name[NAME_LENGTH];
    int id;
    userStruct *user = (userStruct *) arg;
    userCount++;
    systemMessage = "Please enter your phone number(2-20 char)\n";
    write(user->sockfd, systemMessage, strlen(systemMessage));
    while (recv(user->sockfd, name, NAME_LENGTH, 0) <= 0 || strlen(name) < 2 || strlen(name) > NAME_LENGTH) {
        systemMessage = "Failed, Right Usage -> [username] (2-20 char)\n";
        write(user->sockfd, systemMessage, strlen(systemMessage));
    }
    strcpy(user->username, name);
    addUser(user);

    while (1) {
        if (!(recv(user->sockfd, message, MESSAGE_LENGTH, 0) <= 0 || strlen(message) < 2 ||
              strlen(message) >= MESSAGE_LENGTH)) {
            // CREATE
            if (startsWith(message, "-gcreate")) {
                id = findEmptyGroup();
                if (id == -1) {
                    systemMessage = "All groups are opened.\n";
                    write(user->sockfd, systemMessage, strlen(systemMessage));
                } else if (user->inGroup) {
                    sendMessage("You are already in a group first you need to quit.\n", user->userID);
                } else {
                    token = strtok(message, delim);
                    token = strtok(NULL, delim);
                    groups[id]->groupStatus = OPENED;
                    strcpy(groups[id]->groupName, token);
                    token = strtok(NULL, delim);
                    if (token == NULL) {
                        groups[id]->groupStatus = CLOSED;
                        memset(groups[id]->groupName, '\0', sizeof(groups[id]->groupName));
                        systemMessage = "Failed, Right Usage -> [-gcreate] [group name] [group password] \n";
                        write(user->sockfd, systemMessage, strlen(systemMessage));
                    } else {
                        strcpy(groups[id]->groupPassword, token);
                        addUserToGroup(id, user);
                    }
                }
                // WHO AM I
            } else if (startsWith(message, "-whoami")) {
                sprintf(message, "Your phone number is: %s\n", user->username);
                write(user->sockfd, message, strlen(message));
                // JOIN
            } else if (startsWith(message, "-join")) {
                token = strtok(message, delim);
                token = strtok(NULL, delim);
                if (token != NULL) {
                    // If user in id first quit then join new id
                    if (user->inGroup) {
                        quitGroup(user);
                    }

                    bool isPhone;

                    char *str_part;
                    char *phoneControl;
                    phoneControl = malloc(sizeof(char) * (strlen(token) + 1));
                    strcpy(phoneControl, token);

                    long ret = strtol(phoneControl, &str_part, 10);

                    if (ret != 0) {
                        isPhone = true;
                    } else {
                        isPhone = false;
                    }

                    if (isPhone) {
                        // get user id
                        id = getUserID(token);
                        // if there is a user
                        if (id != -1) {
                            strcpy(user->attachedPM,token);
                            puts(user->attachedPM);
                            user->isAttached = true;
                            sendMessage("You can send your private message now with [-send] [message]\n", user->userID);

                        } else {
                            sendMessage("Phone number is wrong try again\n", user->userID);
                        }
                    }
                        // Group join
                    else {
                        id = getGroupID(token);
                        if (id != -1) {
                            if (groups[id]->groupStatus == OPENED) {
                                token = strtok(NULL, delim);
                                if (token == NULL) {
                                    sendMessage("Need Password, Right Usage -> [-join] [group name] [group password] \n", user->userID);
                                } else if (strcmp(token, groups[id]->groupPassword) != 0) {
                                    sendMessage("Wrong Password \n", user->userID);
                                } else {
                                    user->isAttached = false;
                                    addUserToGroup(id, user);
                                }
                            }
                        } else {
                            sendMessage("Group name is wrong try again\n", user->userID);
                        }
                    }
                } else {
                    sendMessage("Enter phone number or group name \n", user->userID);
                }
                // EXIT
            } else if (startsWith(message, "-exit")) {
                token = strtok(message, delim);
                token = strtok(NULL, delim);
                if (token != NULL) {
                    if (!user->inGroup) {
                        sendMessage("You are not in a group \n", user->userID);
                    } else if (strcmp(token, groups[user->groupID]->groupName) != 0) {
                        sendMessage("You entered wrong group name \n", user->userID);
                    } else {
                        quitGroup(user);
                        strcat(jsonFormatted, name);
                        strcat(jsonFormatted, ": left from group \n");
                        sendMessageToGroup(jsonFormatted, user->groupID, user->userID);
                        sendMessage("You quit from your group \n", user->userID);
                    }
                } else {
                    sendMessage("-exit", user->userID);
                }

            } else if (startsWith(message, "-send")) {
                if (user->inGroup) {

                    for (int k = 0; k < sizeof(message) - 5; k++) {
                        message[k] = message[k + 5];
                    }
                    strcat(jsonFormatted, name);
                    strcat(jsonFormatted, ":");
                    strcat(jsonFormatted, message);
                    strcat(jsonFormatted, "\n");
                    sendMessageToGroup(jsonFormatted, id, user->userID);
                } else if (user->isAttached) {

                    for (int k = 0; k < sizeof(message) - 5; k++) {
                        message[k] = message[k + 5];
                    }
                    strcat(jsonFormatted, "PRIVATE MESSAGE -> ");
                    strcat(jsonFormatted, name);
                    strcat(jsonFormatted, ":");
                    strcat(jsonFormatted, message);
                    strcat(jsonFormatted, "\n");
                    id = getUserID(user->attachedPM);
                    sendMessage(jsonFormatted, id);
                } else {
                    sendMessage("You are not in a group \n", user->userID);
                }
            } else {
                sendMessage("Wrong command \n", user->userID);
            }
        }
        memset(message, '\0', sizeof(message));
        memset(jsonFormatted, '\0', sizeof(jsonFormatted));
    }
}

void initGroups() {
    for (int i = 0; i < MAX_GROUP; i++) {
        groupStruct *group = malloc(sizeof(groupStruct));
        group->groupStatus = CLOSED;
        strcpy(group->groupPassword, "");

        for (int j = 0; j < GROUP_CAPACITY; j++) {
            group->usersInGroup[j] = -1;
        }
        groups[i] = group;

    }
}

int main() {

    struct sockaddr_in serverAdr, clientAdr;

    serverAdr.sin_family = AF_INET;
    serverAdr.sin_addr.s_addr = INADDR_ANY;
    serverAdr.sin_port = htons(3205);

    int socket_desc;

    //create socket and check
    socket_desc = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_desc != -1) {
        puts("Socket Created\n");
    } else {
        puts("Socket Error\n");
    }

    int b = bind(socket_desc, (struct sockaddr *) &serverAdr, sizeof(serverAdr));
    if (b >= 0)
        puts("Binded Successfully\n");
    else
        puts("Binding Error\n");

    int l = listen(socket_desc, MAX_USER);
    if (l >= 0)
        puts("Listening...\n");
    else
        puts("Not listening..\n");

    initGroups();

    pthread_t tid;

    int acc;
    while (1) {
        socklen_t clientLen = sizeof(clientAdr);
        acc = accept(socket_desc, (struct sockaddr *) &clientAdr, &clientLen);
        if (userCount < MAX_USER) {
            userStruct *user = ((userStruct *) malloc(sizeof(userStruct)));
            user->address = clientAdr;
            user->sockfd = acc;
            user->inGroup = false;
            pthread_create(&tid, NULL, &connection, (void *) user);
            userCount++;
            puts("New User is Joining Now -> ");
        } else {
            puts("Client limit is over, we can't accept new users\n");
            close(acc);
        }
    }
}
