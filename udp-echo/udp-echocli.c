#include "../basic.h"

int client_echo_udp(FILE *fp, int sockfd, const struct sockaddr *p_servaddr, socklen_t servlen);

int main(int argc, char **argv) {

    // Check arguments
    if (argc != 3) {
        printf("Usage: ./udp-echocli <IPv6 ADDRESS> <PORT>\n");
        return -1;
    }

    // Create socket
    int sockfd;
    if ((sockfd = socket(AF_INET6, SOCK_DGRAM, 0)) < 0) {
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

    // Communicate with the server
    if (client_echo_udp(stdin, sockfd, (struct sockaddr *) &servaddr, sizeof(servaddr)) < 0) {
        return -1;
    }

    return 0;
}

int client_echo_udp(FILE *fp, int sockfd, const struct sockaddr *p_servaddr, socklen_t servlen) {
    int n;
    char sendline[MAXLINE];
    char recvline[MAXLINE + 1];
    char buff[INET6_ADDRSTRLEN];
    socklen_t len;
    struct sockaddr *p_replyaddr;
    struct sockaddr_in6 *sa;

    p_replyaddr = malloc(servlen);

    while (fgets(sendline, MAXLINE, fp) != NULL) {
        if (sendto(sockfd, sendline, strlen(sendline), 0, p_servaddr, servlen) < 0) {
            perror("sendto");
            return -1;
        }
        len = servlen;
        if ((n = recvfrom(sockfd, recvline, MAXLINE, 0, p_replyaddr, &len)) < 0) {
            perror("recvfrom");
            return -1;
        }

        // Check the server identity
        if ((len != servlen) || memcmp(p_servaddr, p_replyaddr, len) != 0) {
            sa = (struct sockaddr_in6 *) p_replyaddr;
            inet_ntop(AF_INET6, &sa->sin6_addr, buff, INET6_ADDRSTRLEN);
            printf("Ignoring reply from %s\n", buff);
            continue;
        }

        recvline[n] = 0;
        fputs(recvline, stdout);
    }

    return 0;
}
