#include "../basic.h"

void server_echo(int sockfd);

/*
 * Servers that echoes a client through a child process
 */
int main(int argc, char **argv) {

    // Check arguments
    if (argc != 2){
        printf("Usage: ./echosrv <PORT>\n");
        return -1; 
    }

    // Create the socket
    int listenfd;
    if ((listenfd = socket(AF_INET6, SOCK_STREAM, 0)) < 0) {
        perror("socket\n");
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
	if (listen(listenfd, BACKLOG) < 0 ) {
        perror("listen");
        return -1;
    }

    // Wait for client requests
    int connfd;
    struct sockaddr_in6 cliaddr;
    socklen_t clilen = sizeof(cliaddr);
    pid_t childpid;
    while (1) {
        if ((connfd = accept(listenfd, (struct sockaddr *) &cliaddr, &clilen)) < 0) {
            if (errno == EINTR) {
                continue;
            } else {
                perror("accept");
                return -1;
            }
        }

        // Create a child process to serve the client
        if ((childpid = fork()) == 0) {
            close(listenfd);
            server_echo(connfd);
            return 0;
        }

        // Close the connection
        close(connfd);
    }
}

void server_echo(int sockfd) {
    ssize_t n;
    char line[MAXLINE];
    while (1) {
        if ((n = exso_readln(sockfd, line, MAXLINE)) == 0) {
            return; // connection closed by the other end
        }
        exso_writen(sockfd, line, n);
    }
}
