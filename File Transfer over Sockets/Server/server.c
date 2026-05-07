/*
** server.c -- a stream socket server
* By Brian Yoo
*/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdbool.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <ctype.h>

#include <arpa/inet.h>

#define MAXDATASIZE 1025 // max number of bytes we can get at once

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

void toUpperCase(char *str) {
    for (int i = 0; i<strlen(str); i++) {
        str[i] = toupper((unsigned char)str[i]);
    }
}

int main(int argc, char *argv[]) {

    int sockfd, clientfd, numbytes;
    char buf[MAXDATASIZE];
    char rbuf[MAXDATASIZE];
    struct addrinfo hints, *servinfo;
    struct sockaddr_storage clients;
    int rv;
    char s[INET6_ADDRSTRLEN];


    if (argc != 2) {
        fprintf(stderr,"usage: hostname port\n");
        exit(1);
    }

    memset(&hints, 0, sizeof  hints);
    hints.ai_socktype = SOCK_STREAM;

    hints.ai_family = AF_UNSPEC;
    hints.ai_flags = AI_PASSIVE;

    if ( (rv = getaddrinfo(NULL, argv[1], &hints, &servinfo)) != 0) {
        fprintf(stderr, "getadderinfo: %s\n", gai_strerror(rv));
        return 1;
    }

    if ((sockfd=socket(servinfo->ai_family, servinfo->ai_socktype, 0)) == -1){
        perror("Server: socket");
    }

    if (bind(sockfd, servinfo->ai_addr, servinfo->ai_addrlen) != 0) {
        perror("Server: bind");
    }
    freeaddrinfo(servinfo);

    printf("Serial Server on host 0.0.0.0/0.0.0.0 is listening on port %s\n", argv[1]);

    listen(sockfd, 5);
    printf("Serial Server starting, listening on port %s\n", argv[1]);

    while (1) {
        socklen_t size = sizeof(clients);

        if ((clientfd = accept(sockfd, (struct sockaddr *)&clients, &size)) == -1) {
            perror("Server: accept");
            exit(1);
        }

        inet_ntop(clients.ss_family, get_in_addr((struct sockaddr *)&clients), s, sizeof s);
        printf("Recived Connection request from %s\n", s);
        printf("**************************************************\n");
        printf("Now listening for incoming messages. . .\n");
        while (1) {
            memset(buf, 0, sizeof buf);

            numbytes = recv(clientfd, buf, MAXDATASIZE, 0);
            printf("Received the following message from client:\n");

            buf[numbytes] = '\0';

            if (strcmp(buf, "exit") == 0) {
                printf("exit\n");
                printf("Client finished, now waiting to service another client. . .\n");
                printf("**************************************************\n");
                break;
            }

            printf("\"%s\"\n", buf);

            if (strncmp(buf, "iWant", 5) == 0) {
                char sub[numbytes-6];
                strncpy(sub, buf + 6, numbytes-6);
                sub[numbytes-6] = '\0';

                FILE *file = fopen(sub, "rb");
                size_t sendTime = 0;

                if (file == NULL) {
                    perror("file not found");
                    if (send(clientfd, &sendTime, sizeof(sendTime), 0) == -1) {
                        perror("send");
                    }
                }else{
                    while ((fread(rbuf,1, MAXDATASIZE, file)) > 0){
                        sendTime ++;
                    }
                    file = fopen(sub, "rb");
                    printf("%s  %zu\n", sub, sendTime);

                    if (send(clientfd, &sendTime, sizeof(sendTime), 0) == -1) {
                        perror("send");
                        exit(1);
                    }

                    memset(buf, 0, MAXDATASIZE);
                    size_t bytesRead = 0;
                    while ((bytesRead = fread(buf, 1, MAXDATASIZE, file)) > 0) {
                        if (send(clientfd, buf, bytesRead, 0) == -1) {
                            perror("send");
                            exit(1);
                        }
                        memset(buf, 0, MAXDATASIZE);
                    }
                    fclose(file);
                }

            }else if (strncmp(buf, "uTake", 5) == 0) {

                size_t sendTime;
                if (recv(clientfd, &sendTime, sizeof(sendTime), 0) == -1) {
                    perror("recv");
                    exit(1);
                }
                printf("%zu\n", sendTime);

                if (sendTime != 0) {
                    char location[MAXDATASIZE];
                    memset(location, 0, sizeof location);

                    ssize_t len;
                    if ((len = recv(clientfd, &location, MAXDATASIZE, 0)) == -1) {
                        perror("recv");
                        exit(1);
                    }
                    location[len] = '\0';

                    printf("%s\n", location);

                    int i = 0;
                    int nameSize = 0;
                    while (buf[numbytes-i] != '/'){
                        nameSize++;
                        i++;
                    }
                    char name[nameSize];
                    strncpy(name, buf + (numbytes - nameSize), nameSize);
                    name[nameSize] = '\0';
                    strcat(location, name);

                    printf("%s, %s, %d\n", location, name, nameSize);

                    FILE* newFile = fopen(location, "wb");
                    if (newFile == NULL) {
                        perror("Error opening file");
                    }else {
                        for (int i = 0; i < sendTime; i++) {
                            memset(buf, 0, MAXDATASIZE);
                            ssize_t bytesRec;
                            if ((bytesRec = recv(clientfd, buf, MAXDATASIZE, 0)) == -1) {
                                perror("recv");
                                exit(1);
                            }
                            fwrite(buf, 1, bytesRec, newFile);
                        }

                        printf("file transfer compeleted successfully\n");
                    }

                    fclose(newFile);

                }

            }

        }
        close(clientfd);

    }

    close(sockfd);

    return 0;
}
