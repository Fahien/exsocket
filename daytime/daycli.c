#include "../basic.h"

/*
 * Client that interacts with a server to receive day and time
 */
int main(int argc, char **argv) {
    // Check arguments
    if (argc != 3) {
        printf("Usage: ./daycli <IPv6 ADDRESS> <PORT>\n");
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

    // Convert the IPv6 address in the network representation
    if (inet_pton(AF_INET6, argv[1], &servaddr.sin6_addr) <= 0) {
        perror("inet_pton");
        return -1;
    }

    // Connect to the server
    if (connect(sockfd, (struct sockaddr *) &servaddr, sizeof(servaddr)) < 0) {
        perror("connect");
        return -1;
    }

    // Get the local address
    struct sockaddr_in6 cliaddr;
    socklen_t clilen = sizeof(cliaddr);
    bzero(&cliaddr, clilen);
    if (getsockname(sockfd, (struct sockaddr *) &cliaddr, &clilen) < 0) {
        perror("getsockname");
        return -1;
    }
    // Convert the IPv6 address in the host representation
    char clistr[INET6_ADDRSTRLEN];
    inet_ntop(AF_INET6, &cliaddr.sin6_addr, clistr, INET6_ADDRSTRLEN);
    printf("The current address is %s:%d\n", clistr, ntohs(cliaddr.sin6_port));

    // Read from the socket buffer
    int n;
    char recvline[MAXLINE + 1];
    if ((n = read(sockfd, recvline, MAXLINE)) > 0) {
        recvline[n] = 0;
        fputs(recvline, stdout);
    }
    if (n < 0) {
        perror("read");
        return -1;
    }

    // Send acknowledgement
    char *ack = "Message received\n";
    if ((n = write(sockfd, ack, strlen(ack))) < 0) {
        perror("write");
        return -1;
    }

    // Close the socket
    close(sockfd);

    return 0;
}
