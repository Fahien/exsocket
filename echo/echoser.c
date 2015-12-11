#include "../basic.h"

/*
 * Servers that echoes a Client using I/O Multiplexing
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

    // Initialize arguments for the select function
    int maxfd = listenfd;
    
    // Initialize client array
    int client[FD_SETSIZE];
    for (int i = 0; i < FD_SETSIZE; i++) {
        client[i] = -1;
    }
    
    // Initialize all descriptors
    fd_set rset;
    fd_set allset;
    FD_ZERO(&allset);
    FD_SET(listenfd, &allset);

    // Ready descriptors
    int ready;
    int connfd;
    int sockfd;
    int i;
    int n;
    struct sockaddr_in6 cliaddr;
    socklen_t clilen = sizeof(cliaddr);
    char buff[MAXLINE];
    while (1) {
        rset = allset;
        if ((ready = select(maxfd + 1, &rset, NULL, NULL, NULL)) < 0) {
            perror("select");
            return -1;
        }

        // Check for new client
        if (FD_ISSET(listenfd, &rset)) {
            if ((connfd = accept(listenfd, (struct sockaddr *) &cliaddr, &clilen)) < 0) {
                perror("accept");
                return -1;
            }
            // Save connected socket descriptor in the client array
            for (i = 0; i < FD_SETSIZE; i++) {
                if (client[i] < 0) {
                    client[i] = connfd;
                    break;
                }
            }

            // Ragequit on too many requests
            if (i == FD_SETSIZE) {
                printf("Too many clients\n");
                return -1;
            }

            // Add connfd to allset
            FD_SET(connfd, &allset);
            if (connfd > maxfd) {
                maxfd = connfd;
            }

            // Check whether there aren't other ready sockets
            if (--ready <= 0) {
                continue;
            }
        }

        // Serve clients
        for (i = 0; i < FD_SETSIZE; i++) {
            if ((sockfd = client[i]) < 0) {
                continue;
            }

            if (FD_ISSET(sockfd, &rset)) {
                if ((n = exso_readln(sockfd, buff, MAXLINE)) < 0) {
                    perror("exso_readln");
                }
                else if (n == 0) {
                    printf("A client is gone\n");
                    close(sockfd);
                    FD_CLR(sockfd, &allset);
                    client[i] = -1;
                } else {
                    if (exso_writen(sockfd, buff, n) < 0) {
                        perror("exso_writen");
                    }
                }

                // Check whether there aren't other ready sockets
                if (--ready <= 0) {
                    break;
                }
            }
        }
    }
}
