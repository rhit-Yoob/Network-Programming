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
    struct addrinfo hints, *servinfo;
    struct sockaddr_storage clients;
    int rv;
    char s[INET6_ADDRSTRLEN];


    if (argc != 3) {
        fprintf(stderr,"usage: server hostname port\n");
        exit(1);
    }

    memset(&hints, 0, sizeof  hints);
    if (strcmp(argv[1], "-t")==0) {
        hints.ai_socktype = SOCK_STREAM;
    }else if (strcmp(argv[1], "-u")==0) {
        hints.ai_socktype = SOCK_DGRAM;
    }
    hints.ai_family = AF_UNSPEC;
    hints.ai_flags = AI_PASSIVE;

    if ( (rv = getaddrinfo(NULL, argv[2], &hints, &servinfo)) != 0) {
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

    printf("Serial Server on host 0.0.0.0/0.0.0.0 is listening on port %s\n", argv[2]);

    listen(sockfd, 5);
    printf("Serial Server starting, listening on port %s\n", argv[2]);
    int i = 0;

    if (strcmp(argv[1], "-t")==0) {
        while (1) {
            socklen_t size = sizeof(clients);

            if ((clientfd = accept(sockfd, (struct sockaddr *)&clients, &size)) == -1) {
                perror("Server: accept");
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

                if (strcmp(buf, ";;;") == 0) {
                    printf(";;;\n");
                    printf("Client finished, now waiting to service another client. . .\n");
                    printf("**************************************************\n");
                    break;
                }
                i++;

                printf("\"%s\"\n", buf);

                toUpperCase(buf);
                char sendbuf[MAXDATASIZE + 20];
                memset(sendbuf, 0, sizeof sendbuf);
                snprintf(sendbuf, sizeof sendbuf, "%d %s", i, buf);
                int len = strlen(sendbuf);
                if (send(clientfd, sendbuf, len, 0) == -1) {
                    perror("send");
                }

                printf("Now sending message %d back having changed the string to upper case. . .\n", i);
            }
            close(clientfd);

        }
    }else if (strcmp(argv[1], "-u")==0) {
        while (1) {
            socklen_t size = sizeof(clients);
            printf("Now listening for incoming messages. . .\n");
            while (1) {
                i++;
                memset(buf, 0, sizeof buf);

                numbytes = recvfrom(sockfd, buf, MAXDATASIZE, 0, (struct sockaddr *)&clients, &size);

                printf("Received the following message from client:\n");

                buf[numbytes] = '\0';
                printf("\"%s\"\n", buf);

            }

            close(clientfd);

        }
    }
    close(sockfd);

    return 0;
}