/*
** proxy.c -- a proxy server
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

void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int main(int argc, char *argv[]) {

    int sockfd, clientfd, numbytes;
    char buf[MAXDATASIZE];
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
        exit(1);
    }

    if (bind(sockfd, servinfo->ai_addr, servinfo->ai_addrlen) != 0) {
        perror("Server: bind");
        exit(1);
    }
    freeaddrinfo(servinfo);

    printf("Proxy on host 0.0.0.0/0.0.0.0 is listening on port %s\n", argv[1]);

    listen(sockfd, 5);
    printf("Proxy starting, listening on port %s\n", argv[1]);

    while (1) {
        socklen_t size = sizeof(clients);

        if ((clientfd = accept(sockfd, (struct sockaddr *)&clients, &size)) == -1) {
            perror("Server: accept");
            exit(1);
        }

        int count = 0;
        pid_t pid = fork();

        if (pid == 0) {
            //child
            count++;
            if (count > 100) {
                close(clientfd);
                exit(2);
            }
            inet_ntop(clients.ss_family, get_in_addr((struct sockaddr *)&clients), s, sizeof s);
            printf("Recived Connection request from %s\n", s);
            printf("**************************************************\n");
            memset(buf, 0, MAXDATASIZE);
            numbytes = recv(clientfd, buf, MAXDATASIZE, 0);
            buf[numbytes] = '\0';

            printf("Received the following message from client:\n");
            printf("%s\n", buf);

            char sendbuf[MAXDATASIZE];
            memset(sendbuf, 0, sizeof(sendbuf));

            if (numbytes <= 0) {
                strcpy(sendbuf, "HTTP/1.0 400 Bad Request\r\n\r\n");
                send(clientfd, sendbuf, strlen(sendbuf), 0);
                exit(1);
            }

            char action[4];
            memset(action, 0, sizeof(action));
            memcpy(action, buf, 3);
            action[3] = '\0';

            if (strcmp(action, "GET") == 0) {
                printf("get\n");

                int space = 0;
                int slash = 0;
                int start = 0;
                int end = 0;

                for (int i = 0; i<numbytes; i++) {
                    if (buf[i] == ' ') {
                        space++;
                        if (space == 2) {
                            end = i;
                            break;
                        }
                    }else if (buf[i] == '/') {
                        slash++;
                        if (slash == 3) {
                            start = i;
                        }
                    }
                }

                char path[300];
                memset(path, 0, sizeof(path));
                memcpy(path, buf + start, end - start);
                printf("path: %s\n", path);

                start = 0;
                end = 0;
                int pStart = 0;
                slash = 0;
                space = 0;
                for (int i = 0; i<numbytes; i++) {
                    if (buf[i] == '/') {
                        slash++;
                        if (slash == 2) {
                            start = i+1;
                        }else if (slash == 3) {
                            end = i;
                            break;
                        }
                    }else if (buf[i] == ':') {
                        space++;
                        if (space == 2) {
                            pStart = i;
                        }
                    }
                }

                char host[300];
                memset(host, 0, sizeof(host));
                char port[300];
                memset(port, 0, sizeof(port));
                if (pStart == 0) {
                    strcpy(port, "80");
                    memcpy(host, buf + start, end - start);
                    printf("host: %s\n", host);
                    printf("port: %s\n", port);
                }else {
                    memcpy(port, buf + pStart, end - pStart);
                    memcpy(host, buf + start, pStart - start);
                    printf("host: %s\n", host);
                    printf("port: %s\n", port);
                }
                snprintf(sendbuf, sizeof(sendbuf), "GET %s HTTP/1.0\r\nHost: %s:%s\r\nConnection: close\r\nUser-Agent: Mozilla/5.0\r\n\r\n", path, host, port);

                struct addrinfo cHints, *cServinfo, *o;
                memset(&cHints, 0, sizeof  cHints);
                cHints.ai_family = AF_UNSPEC;
                cHints.ai_socktype = SOCK_STREAM;


                int crv;

                if ( (crv = getaddrinfo(host, port, &cHints, &cServinfo)) != 0) {
                    fprintf(stderr, "getadderinfo: %s\n", gai_strerror(crv));
                    strcpy(sendbuf, "HTTP/1.0 400 Bad Request\r\n\r\n");
                    send(clientfd, sendbuf, strlen(sendbuf), 0);
                    exit(1);
                    return 1;
                }

                int csockfd;
                for (o = cServinfo; o != NULL; o = o->ai_next) {
                    csockfd = socket(o->ai_family, o->ai_socktype, o->ai_protocol);

                    if (csockfd == -1) {
                        perror("client: socket");
                        continue;
                    }

                    crv = connect(csockfd, o->ai_addr, o->ai_addrlen);

                    if (crv == -1) {
                        perror("client: connect");
                        continue;
                    }

                    //all is well
                    break;
                }
                freeaddrinfo(cServinfo);

                printf("client: connected\n");
                printf("sending: %s\n", sendbuf);
                if (send(csockfd, sendbuf, strlen(sendbuf), 0) == -1) {
                    perror("send");
                    exit(1);
                }

                memset(buf, 0, MAXDATASIZE);
               // numbytes = recv(csockfd, buf, MAXDATASIZE, 0);
                //buf[numbytes] = '\0';

                //printf("Received the following message from client:\n");
                //printf("%s\n", buf);

                while ((numbytes = recv(csockfd, buf, MAXDATASIZE, 0)) > 0) {
                    if (send(clientfd, buf, numbytes, 0) == -1) {
                        perror("send");
                        exit(1);
                    }
                }
                printf("done\n");
                close(csockfd);

            }else {
                printf("501 Not Implemented\n");
                strcpy(sendbuf, "HTTP/1.0 501 Not Implemented\r\n\r\n");
                if (send(clientfd, sendbuf, strlen(sendbuf), 0) == -1) {
                    perror("send");
                    exit(1);
                }

            }

            close(sockfd);
            close(clientfd);
            exit(0);
        }else {
            //parent
            close(clientfd);
        }
    }

    close(sockfd);
    return 0;
}