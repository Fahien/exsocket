#include "../basic.h"

#define MAXNICK 32

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
    
    // Initialize the array of clients
    int client[FD_SETSIZE];
    for (int i = 0; i < FD_SETSIZE; i++) {
        client[i] = -1;
    }

    // Initialize the array of client nicknames
    int nicknames[FD_SETSIZE][MAXNICK];
    
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
    char temp[MAXLINE];
    char addrbuff[INET6_ADDRSTRLEN];
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
            // Convert IPv6 address to presentation
            inet_ntop(AF_INET6, &cliaddr.sin6_addr, addrbuff, INET6_ADDRSTRLEN);
            printf("Connected client from %s:%d\n", addrbuff, ntohs(cliaddr.sin6_port));

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
                    // Convert IPv6 address to presentation
                    inet_ntop(AF_INET6, &cliaddr.sin6_addr, addrbuff, INET6_ADDRSTRLEN);
                    printf("Disconnected client from %s:%d\n", addrbuff, ntohs(cliaddr.sin6_port));
                    
                    close(sockfd);
                    FD_CLR(sockfd, &allset);
                    client[i] = -1;
                    bzero(nicknames[i], MAXNICK);
                } else {
                    if (nicknames[i][0] == 0) {
                        if (strncmp(buff, "/nickname", 9) == 0) {
                            sscanf(buff, "/nickname %s\n", nicknames[i]);
                            sprintf(buff, "%s has joined the chat\n", nicknames[i]);
                        } else {
                            sprintf(buff, "Error, expecting /nickname <NICKNAME>\n");
                            exso_writen(client[i], buff, strlen(buff));
                            continue;
                        }
                    } else {
                        sprintf(temp, "%s: %s", nicknames[i], buff);
                        sprintf(buff, "%s", temp);
                    }
                    for (i = 0; i < FD_SETSIZE; i++) {
                        if (sockfd == client[i] || client[i] == -1) {
                            continue;
                        }
                        if (exso_writen(client[i], buff, strlen(buff)) < 0) {
                            perror("exso_writen");
                        }
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
