#include "../basic.h"

#define BYE_MESSAGE "BYE\n"
#define END_MESSAGE ".\n"

int main (int argc, char** argv)
{
    // Check arguments
    if (argc != 3) {
        fprintf(stderr, "Usage: ./ecommercecli <IP> <PORT>\n");
        return EXIT_FAILURE;
    }

    // Create server sockaddr structure
    struct sockaddr_in servaddr;
    socklen_t servlen = sizeof servaddr;
    memset(&servaddr, 0, servlen);
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(atoi(argv[2]));
    int pton_result = inet_pton(AF_INET, argv[1], &servaddr.sin_addr);
    // Check inet_pton error
    if (pton_result <= 0) {
        fprintf(stderr, "Could not convert address %s\n", argv[1]);
        return EXIT_FAILURE;
    }

    // Create socket
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    // Check socket error
    if (sockfd < 0) {
        perror("Could not create the socket");
        return EXIT_FAILURE;
    }

    // Connect to server
    int conn_result = connect(sockfd, (struct sockaddr*) &servaddr, servlen);
    // Check connect error
    if (conn_result < 0) {
        perror("Could not connect to the server");
        return EXIT_FAILURE;
    }

    // Loop variables
    char buffer[MAXLINE];
    memset(buffer, 0, MAXLINE);

    // Get Welcome message
    int read_result = exso_readln(sockfd, buffer, MAXLINE);
    // Check read error
    if (read_result < 0) {
        perror("Could not read welcome message from server");
        return EXIT_FAILURE;
    }
    printf("%s", buffer);

    // Loop
    while (true) {
        memset(buffer, 0, MAXLINE);
        // Read input
        char* get_result = fgets(buffer, MAXLINE, stdin);
        // Check read error
        if (get_result == NULL) {
            fprintf(stderr, "Could not read from standard input\n");
            break;
        }

        // Send command
        int write_result = exso_writen(sockfd, buffer, strlen(buffer));
        // Check write error
        if (write_result < 0) {
            perror("Could not write to the server");
            return EXIT_FAILURE;
        }
        else if (write_result == 0) {
            fprintf(stderr, "Connection terminated by the server\n");
            break;
        }

        // Exit on BYE_MESSAGE
        bool bye = memcmp(buffer, BYE_MESSAGE, strlen(BYE_MESSAGE)) == 0;
        if (bye) break;

        bool is_not_end_message = true;
        // Get reply until END_MESSAGE
        do {
            memset(buffer, 0, MAXLINE);
            int read_result = exso_readln(sockfd, buffer, MAXLINE);
            // Check read error 
            if (read_result < 0) {
                perror("Could not read from the server");
                return EXIT_FAILURE;
            }
            else if (read_result == 0) {
                perror("Connection terminated\n");
                goto close;
            }
            printf("%s", buffer);

            is_not_end_message = memcmp(buffer, END_MESSAGE, strlen(END_MESSAGE)) != 0;
        } while (is_not_end_message);
    } // Endloop

    close:;
    // Close socket
    int close_result = close(sockfd);
    if (close_result < 0) {
        perror("Could not close the socket");
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
