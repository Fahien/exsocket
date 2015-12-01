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

    // Convert IPv6 address in the network representation
    if (inet_pton(AF_INET6, argv[1], &servaddr.sin6_addr) <= 0) {
        perror("inet_pton");
        return -1;
    }

    // Connect to the server
    if (connect(sockfd, (struct sockaddr *) &servaddr, sizeof(servaddr)) < 0) {
        perror("connect");
        return -1;
    }

    // Read from the socket buffer
    int n;
    char recvline[MAXLINE + 1];
    while ((n = read(sockfd, recvline, MAXLINE)) > 0) {
        recvline[n] = 0;
        fputs(recvline, stdout);
    }
    if (n < 0) {
        perror("read");
        return -1;
    }

    // Close the socket
    close(sockfd);

    return 0;
}
