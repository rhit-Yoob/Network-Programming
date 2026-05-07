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
    char buf[MAXDATASIZE];
    char rbuf[MAXDATASIZE];
    struct addrinfo hints, *servinfo, *p;
    int rv;
    char s[INET6_ADDRSTRLEN];

    if (argc != 3) {
        fprintf(stderr,"usage: client hostname port\n");
        exit(1);
    }

    memset(&hints, 0, sizeof  hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;


    if ( (rv = getaddrinfo(argv[1], argv[2], &hints, &servinfo)) != 0) {
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
    printf("Client has requested to start connection with host %s on port %s\n", s, argv[2]);
    printf("*****************************************************************\n");
    printf("Connection established, now waiting for user input. . . \n");
    freeaddrinfo(servinfo);

    char* line = NULL;
    size_t size = 0;
    size_t len;

    while (readLine(&line, &size, &len)) {
        if (strcmp(line, "exit") == 0) {
            if (send(sockfd, line, len, 0) == -1) {
                perror("send");
            }
            printf("see ya!\n");
            break;
        }

        if (strncmp(line, "iWant", 5) == 0) {
            if (send(sockfd, line, len, 0) == -1) {
                perror("send");
            }

            size_t sendTime;
            if (recv(sockfd, &sendTime, sizeof(sendTime), 0) == -1) {
                perror("recv");
                exit(1);
            }

            if (sendTime == 0) {
                printf("Failure: Cannot find that file!\n");
            }else {
                char* location = NULL;
                size_t location_size = 0;
                size_t location_len;

                printf("%zu\n", sendTime);
                printf("What directory would you like to save this file?\n");
                readLine(&location, &location_size, &location_len);

                int i = 0;
                int nameSize = 0;
                while (line[len-i] != '/'){
                    nameSize++;
                    i++;
                }

                char name[nameSize];
                strncpy(name, line + (len - nameSize), nameSize);
                name[nameSize] = '\0';
                strcat(location, name);

                printf("%s, %s, %d\n", location, name, nameSize);

                printf("file transfer started...\n");
                FILE* newFile = fopen(location, "wb");
                if (newFile == NULL) {
                    perror("Error opening file");
                    exit(1);
                }

                for (int i = 0; i < sendTime; i++) {
                    memset(buf, 0, MAXDATASIZE);
                    ssize_t bytesRec;
                    if ((bytesRec = recv(sockfd, buf, MAXDATASIZE, 0)) == -1) {
                        perror("recv");
                        exit(1);
                    }
                    fwrite(buf, 1, bytesRec, newFile);
                }

                printf("file transfer compeleted successfully\n");

                fclose(newFile);
            }

        }else if (strncmp(line, "uTake", 5) == 0) {
            if (send(sockfd, line, len, 0) == -1) {
                perror("send");
            }

            char sub[len-6];
            strncpy(sub, line + 6, len-6);
            sub[len-6] = '\0';

            FILE *file = fopen(sub, "rb");
            size_t sendTime = 0;

            if (file == NULL) {
                printf("file not found\n");
                if (send(sockfd, &sendTime, sizeof(sendTime), 0 ) == -1) {
                    perror("send");
                }
            }else {
                while ((fread(rbuf,1, MAXDATASIZE, file)) > 0){
                    sendTime ++;
                }
                printf("%zu\n", sendTime);
                file = fopen(sub, "rb");

                if (send(sockfd, &sendTime, sizeof(sendTime), 0) == -1) {
                    perror("send");
                    exit(1);
                }

                printf("What directory would you like to save this file?\n");
                char* location = NULL;
                size_t location_size = 0;
                size_t location_len;

                readLine(&location, &location_size, &location_len);
                printf("%s\n", location);
                if (send(sockfd, location, location_len, 0) == -1) {
                    perror("send");
                    exit(1);
                }

                printf("file transfer started...\n");

                memset(buf, 0, MAXDATASIZE);
                size_t bytesRead = 0;
                while ((bytesRead = fread(buf, 1, MAXDATASIZE, file)) > 0) {
                    if (send(sockfd, buf, bytesRead, 0) == -1) {
                        perror("send");
                        exit(1);
                    }
                    memset(buf, 0, MAXDATASIZE);
                }
                printf("file transfer compeleted successfully\n");
                fclose(file);

            }

        }else {
            printf("That just aint right!\n");
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
