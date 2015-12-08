#include "../basic.h"

#define OP_LEN 3 // + - * / mod

int client_cal_udp(FILE *fp, int sockfd, struct sockaddr *servaddr, socklen_t servlen);

int main(int argc, char **argv) {

    // Check the arguments
    if (argc != 3) {
        printf("Usage: ./calcli <IPv6 ADDRESS> <PORT>\n");
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
    servaddr.sin6_port = htons(atoi(argv[2]));

    // Convert the IPv6 address to the network representation
    if (inet_pton(AF_INET6, argv[1], &servaddr.sin6_addr) < 0) {
        perror("inet_pton");
        return -1;
    }

    // Communicate with the server
    if (client_cal_udp(stdin, sockfd, (struct sockaddr *) &servaddr, sizeof(servaddr)) < 0) {
        return -1;
    }

    // Close the socket
    if (close(sockfd) < 0) {
        perror("close");
        return -1;
    }

    return 0;
}

int client_cal_udp(FILE *fp, int sockfd, struct sockaddr *servaddr, socklen_t servlen) {

    // Ask for user input
    int n;
    char buff[MAXLINE];
    int a = 0;
    int b = 0;
    char op[OP_LEN] = "+";

    struct sockaddr *replyaddr;
    replyaddr = malloc(servlen);
    socklen_t len;

    while (fgets(buff, MAXLINE, fp) != NULL) {
        // Check user input
        if ((n = sscanf(buff, "%d %s %d", &a, op, &b)) == 3) {
            // Send the message to the server
            if (sendto(sockfd, buff, strlen(buff), 0, servaddr, servlen) < 0) {
                perror("sendto");
                return -1;
            }

            // Wait for the reply
            len = servlen;
            if ((n = recvfrom(sockfd, buff, MAXLINE, 0, replyaddr, &len)) < 0) {
                perror("recvfrom");
                return -1;
            }

            // Check the server identity
            if ((len != servlen) || memcmp(servaddr, replyaddr, len) != 0) {
                printf("Ignoring reply from unknown peer\n");
                continue;
            }

            // Print the reply            
            buff[n] = 0;
            printf("%s\n", buff);
        } else {
                printf("Invalid input\n");
        }
    }
    return 0;
}
