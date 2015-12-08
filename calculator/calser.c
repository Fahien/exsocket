#include "../basic.h"

#define OP_LEN 3 // + - * / mod

int server_cal_udp(int sockfd, struct sockaddr *cliaddr, socklen_t clilen);

int main(int argc, char **argv) {

    // Check the arguments
    if (argc != 2) {
        printf("Usage: ./calser <PORT>\n");
        return -1;
    }

    // Create the socket
    int sockfd;
    if ((sockfd = socket(AF_INET6, SOCK_DGRAM, 0)) < 0) {
        perror("socket");
        return -1;
    }

    // Initialize the server sockaddr structure
    struct sockaddr_in6 servaddr;
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin6_family = AF_INET6;
    servaddr.sin6_port = htons(atoi(argv[1]));
    servaddr.sin6_addr = in6addr_any;

    if (bind(sockfd, (struct sockaddr *) &servaddr, sizeof(servaddr)) < 0) {
        perror("bind");
        return -1;
    }

    // Serve the clients
    struct sockaddr_in6 cliaddr;
    if (server_cal_udp(sockfd, (struct sockaddr *) &cliaddr, sizeof(cliaddr)) < 0) {
        return -1;
    }

    return 0;
}

int server_cal_udp(int sockfd, struct sockaddr *cliaddr, socklen_t clilen) {

    char buff[MAXLINE];
    socklen_t len;

    int a;
    int b;
    char op[OP_LEN];

    while (1) {
        len = clilen;
        if (recvfrom(sockfd, buff, MAXLINE, 0, cliaddr, &len) < 0) {
            perror("recvfrom");
            return -1;
        }

        if (sscanf(buff, "%d %s %d", &a, op, &b) == 3) {
            if (strcmp(op, "+") == 0) {
                sprintf(buff, "%d", a + b);
            } else if (strcmp(op, "-") == 0) {
                sprintf(buff, "%d", a - b);
            } else if (strcmp(op, "*") == 0) {
                sprintf(buff, "%d", a * b);
            } else if (strcmp(op, "/") == 0) {
                if (b != 0) {
                    sprintf(buff, "%d", a / b);
                } else {
                    sprintf(buff, "Undefined");
                }
            } else if (strcmp(op, "mod") == 0) {
                sprintf(buff, "%d", a % b);
            } else {
                sprintf(buff, "Invalid operator");
            }
        } else {
            sprintf(buff, "Invalid message");
        }

        if (sendto(sockfd, buff, strlen(buff), 0, cliaddr, len) < 0) {
            perror("sendto");
            return -1;
        }
    }

    return 0;
}
