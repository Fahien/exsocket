#include	"../basic.h"

int client_echo(FILE *fp, int sockfd); 

/*
 * Client that queries an echo Server
 */
int main(int argc, char **argv) {

    // Check arguments
    if (argc != 3) {
        printf("Usage: ./echocli <IPv6 ADDRESS> <PORT>\n");
        return -1;
    }

    // Create the socket
    int sockfd;
    if ((sockfd = socket(AF_INET6, SOCK_STREAM, 0)) < 0) {
        perror("socket");
        return -1;
    }

    // Initialize server sockaddr structure
    struct sockaddr_in6 servaddr;
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin6_family = AF_INET6;
    servaddr.sin6_port = htons(atoi(argv[2]));

    // Convert the IPv6 Address in the network representation
    if (inet_pton(AF_INET6, argv[1], &servaddr.sin6_addr) <= 0) {
        perror("inet_pton");
        return -1;
    }

    // Connect to the server
    if (connect(sockfd, (struct sockaddr *) &servaddr, sizeof(servaddr)) < 0) {
        perror("connect");
        return -1;
    }

    // Communicate with the server
    if (client_echo(stdin, sockfd) < 0) {
        return -1;
    }

    return 0;
}


int client_echo(FILE *fp, int sockfd) {
    
    fd_set readset;
    FD_ZERO(&readset);
    int maxfd;

    int n;
    char sendline[MAXLINE];
    char recvline[MAXLINE];

    while (1) {
        // Initialize the arguments for the select function
        FD_SET(fileno(fp), &readset);
        FD_SET(sockfd, &readset);
        maxfd = MAX(fileno(fp), sockfd) + 1;

        if (select(maxfd, &readset, NULL, NULL, NULL) < 0) {
            perror("select");
            return -1;
        }

        // Check whether the socket is ready for reading (a message has arrived)
        if (FD_ISSET(sockfd, &readset)) {
            if ((n = exso_readln(sockfd, recvline, MAXLINE)) < 0) {
                perror("exso_readln");
                return -1;
            }
            if (n == 0) {
                printf("Server terminated prematurely\n");
                return -1;
            }
            fputs(recvline, stdout);
        }

        // Check whether fp is ready for reading
        if (FD_ISSET(fileno(fp), &readset)) {
            if (fgets(sendline, MAXLINE, fp) == NULL) {
                return 0;
            }
            if (exso_writen(sockfd, sendline, strlen(sendline)) < 0) {
                perror("exso_writen");
                return -1;
            }
        }
    }

    return 0;
}
