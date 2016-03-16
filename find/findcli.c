#include "../basic.h"

/*
 * Client that reads from a input a <FILE> and a <STRING>
 * then send them to a server which will search within the file
 * the line in which the string appears more times and returns the
 * line number and number of occurrences of the string in that row
 */
int main (int argc, char** argv)
{
    // Check arguments
    if (argc != 3) {
        fprintf(stderr, "Usage: ./findcli address port\n");
        return EXIT_FAILURE;
    }

    // Create server sockaddr structure
    struct sockaddr_in6 servaddr;
    memset(&servaddr, 0, sizeof servaddr);
    servaddr.sin6_family = AF_INET6;
    servaddr.sin6_port = htons(atoi(argv[2]));

    // Convert address to network representation
    int pton_result = inet_pton(AF_INET6, argv[1], &servaddr.sin6_addr);
    // Check for inet_pton error
    if (pton_result == 0) {
        fprintf(stderr, "IPv6 Address not valid\n");
        return EXIT_FAILURE;
    }
    else if (pton_result < 0) {
        perror("inet_pton");
        return EXIT_FAILURE;
    }

    // Get file name
    printf("Please, insert the file name: ");
    char file_name[MAXLINE];
    memset(file_name, 0, MAXLINE);
    scanf("%s", file_name);

    // Get the word to search
    printf("Please, insert the word to search: ");
    char word[MAXLINE];
    memset(word, 0, MAXLINE);
    scanf("%s", word);

    // Construct the buffer to send
    char buffer[MAXLINE];
    memset(buffer, 0, MAXLINE);
    sprintf(buffer, "%s %s\n", file_name, word);

    // Create a socket
    int sockfd = socket(AF_INET6, SOCK_STREAM, 0);
    // Check for socket error
    if (sockfd < 0) {
        perror("socket");
        return EXIT_FAILURE;
    }

    // Connect to the server
    int connect_result = connect(sockfd, (struct sockaddr*) &servaddr, sizeof servaddr);
    // Check for connect error
    if (connect_result < 0) {
        perror("connect");
        return EXIT_FAILURE;
    }

    printf("Attempting to send: %s", buffer);

    // Write the buffer into the connected socket
    int writen_result = exso_writen(sockfd, buffer, strlen(buffer));
    // Check for writen error
    if (writen_result < 0) {
        perror("exso_writen");
        return EXIT_FAILURE;
    }

    // Read the reply from server
    memset(buffer, 0, MAXLINE);
    int readln_result = exso_readln(sockfd, buffer, MAXLINE);
    // Check for readln error
    if (readln_result == 0) {
        fprintf(stderr, "Server terminated prematurely\n");
        return EXIT_FAILURE;
    }
    else if (readln_result < 0) {
        perror("exso_readln");
        return EXIT_FAILURE;
    }

    // Print out the reply
    printf("Received: %s", buffer);

    // Close the socket
    int close_result = close(sockfd);
    // Check for close error
    if (close_result < 0) {
        perror("close");
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
