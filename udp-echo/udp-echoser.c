#include "../basic.h"

int server_echo_udp(int sockfd, struct sockaddr *p_cliaddr, socklen_t clilen);

int main(int argc, char **argv) {

    // Check arguments
    if (argc != 2) {
        printf("Usage: udp-echoser <PORT>\n");
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
    servaddr.sin6_family = AF_INET6,
    servaddr.sin6_addr = in6addr_any;
    servaddr.sin6_port = htons(atoi(argv[1]));
    
    if (bind(sockfd, (struct sockaddr *) &servaddr, sizeof(servaddr)) < 0) {
        perror("bind");
        return -1;
    }

    // Serve the clients
    struct sockaddr_in6 cliaddr;
    if (server_echo_udp(sockfd, (struct sockaddr *) &cliaddr, sizeof(cliaddr)) < 0) {
        return -1;
    }

    return 0;
}

int server_echo_udp(int sockfd, struct sockaddr *p_cliaddr, socklen_t clilen) {
    int n;
    int m;
    socklen_t len;
    char buff[MAXLINE];

    while (1) {
        len = clilen;
        if ((n = recvfrom(sockfd, buff, MAXLINE, 0, p_cliaddr, &len)) < 0) {
            perror("recvfrom");
            return -1;
        }
        if ((m = sendto(sockfd, buff, n, 0, p_cliaddr, len)) != n) {
            if (m < 0) {
                perror("sendto");
            } else {
                printf("Sendto error\n");
            }
            return -1;
        }
    }

    return 0;
}
