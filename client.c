/*
** client.c -- a stream socket client
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

#include <arpa/inet.h>

#define PORT "3490" // the port client will be connecting to

#define MAXDATASIZE 1025 // max number of bytes we can get at once

bool readLine(char** line, size_t* size, size_t* length);

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int main(int argc, char *argv[])
{

    int sockfd; //, numbytes;
    //char buf[MAXDATASIZE];
    struct addrinfo hints, *servinfo, *p;
    int rv;
    char s[INET6_ADDRSTRLEN];

    if (argc != 4) {
        fprintf(stderr,"usage: client hostname port\n");
        exit(1);
    }

    memset(&hints, 0, sizeof  hints);
    hints.ai_family = AF_UNSPEC;
    if (strcmp(argv[1], "-t")==0) {
        hints.ai_socktype = SOCK_STREAM;
    }else if (strcmp(argv[1], "-u")==0) {
        hints.ai_socktype = SOCK_DGRAM;
    }
    if ( (rv = getaddrinfo(argv[2], argv[3], &hints, &servinfo)) != 0) {
        fprintf(stderr, "getadderinfo: %s\n", gai_strerror(rv));
        return 1;
    }

    for (p = servinfo; p != NULL; p = p->ai_next) {
        sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);

        if (sockfd == -1) {
            perror("client: socket");
            continue;
        }

        rv = connect(sockfd, p->ai_addr, p->ai_addrlen);

        if (rv == -1) {
            perror("client: connect");
            continue;
        }

        //all is well
        break;
    }
    //make address readable into s
    inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr), s, sizeof s);
    printf("Client has requested to start connection with host %s on port %s\n", s, argv[3]);
    printf("*****************************************************************\n");
    printf("Connection established, now waiting for user input. . . \n");
    freeaddrinfo(servinfo);

    char* line = NULL;
    size_t size = 0;
    size_t len;

    while (readLine(&line, &size, &len)) {
        printf("Client just read: %s\n", line);

        printf("Sending message to Server. . .\n");
        if (send(sockfd, line, len, 0) == -1) {
            perror("send");
        }

        if (strcmp(line, ";;;") == 0) {
            printf("User entered sentinel of \";;;\", now stopping client\n");
            break;
        }

        if (strcmp(argv[1], "-t")==0) {
            memset(line, 0, MAXDATASIZE);

            if (recv(sockfd, line, MAXDATASIZE, 0) == -1) {
                perror("recv");
            }
            printf("Received response from server of\n");
            printf("%s\n", line);
        }
    }

    printf("**********************************************************************\n");
    printf("Attempting to shut down client sockets and other streams\n");
    close(sockfd);
    printf("Shut down successful... goodbye\n");

    return 0;
}

bool readLine(char** line, size_t* size, size_t* length)
{
    while (1)
    {
        //get line
        printf("prompt > ");
        size_t len = getline(line, size, stdin);

        //handle EOF
        if (len == -1)
            return false;

        if ((*line)[len-1] ==  '\n')
            (*line)[--len] = '\0';

        *length = len;

        if (len == 0)
            continue;

        return len >= 1;
    }
    return false;
}
