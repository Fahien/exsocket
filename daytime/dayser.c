#include "../basic.h"

/*
 * Server that sends day and time to the clients
 */
int main(int argc, char **argv) {

    // Check arguments
    if (argc != 2) {
        printf("Usage: ./dayser <PORT>\n");
        return -1;
    }

    // Create the socket
    int listenfd;
    if ((listenfd = socket(AF_INET6, SOCK_STREAM, 0)) < 0) {
        perror("socket");
        return -1;
    }

    // Initialize server sockaddr structure
    struct sockaddr_in6 servaddr;
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin6_family = AF_INET6;
    servaddr.sin6_addr = in6addr_any;
    servaddr.sin6_port = htons(atoi(argv[1]));

    if ((bind(listenfd, (struct sockaddr *) &servaddr, sizeof(servaddr))) < 0) {
        perror("bind");
        return -1;
    }

    // Convert the socket to a listening socket
    if (listen(listenfd, BACKLOG) < 0) {
        perror("listen");
        return -1;
    }

    // Wait for client requests
    int connfd;
    struct sockaddr_in6 cliaddr;
    socklen_t clilen;
    clilen = sizeof(cliaddr);
    bzero(&cliaddr, clilen);
    int n;
    time_t ticks;
    char buff[MAXLINE];
    while (1) {
        if ((connfd = accept(listenfd, (struct sockaddr *) &cliaddr, &clilen)) < 0) {
            perror("accept");
            return 1;
        }

        // Convert the client address
        inet_ntop(AF_INET6, &cliaddr.sin6_addr, buff, INET6_ADDRSTRLEN);
        printf("Serving new client from %s:%d\n", buff, ntohs(cliaddr.sin6_port));

        // Send the time to the client
        ticks = time(NULL);
        snprintf(buff, sizeof(buff), "%.24s\r\n", ctime(&ticks));
        while ((n = write(connfd, buff, strlen(buff))) < 0);

        // Close the connection
        close(connfd);
    }

    return 0;
}
