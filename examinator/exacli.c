#include "../basic.h"

/*
 * This client reads two integer from the standard input,
 * THRESHOLD and NUMBER, then sends them to the server
 * and wait for its reply.
 */
int main (int argc, char** argv)
{
	// Check arguments
	if (argc != 3) {
		printf("Usage: ./exacli <IPv6 ADDRESS> <PORT>\n");
		return EXIT_FAILURE;
	}

	int threshold;
	int tests_count;
	int scanf_result = 0;

	// Read threshold and tests_count
	while (scanf_result != 2) {
		printf("Insert the threshold and the minimum number of tests separated by a space: ");
		scanf_result = scanf("%d %d", &threshold, &tests_count);
	}

	// Prepare line to send
	char line[MAXLINE];
	memset(line, 0, MAXLINE);
	sprintf(line, "%d %d\n", threshold, tests_count);
	printf("Sending %s", line);

	// Create the socket
	int sockfd = socket(AF_INET6, SOCK_STREAM, 0);
	if (sockfd < 0) {
		perror("socket");
		return EXIT_FAILURE;
	}

	// Initialize server sockaddr structure
	struct sockaddr_in6 servaddr;
	size_t servlen = sizeof(servaddr);
	memset(&servaddr, 0, servlen);
	servaddr.sin6_family = AF_INET6;
	const char* port = argv[2];
	servaddr.sin6_port = htons(atoi(port));

	// Convert the IPv6 Address in the network representation
	int inet_pton_result = inet_pton(AF_INET6, argv[1], &servaddr.sin6_addr);
	if (inet_pton_result <= 0) {
		perror("inet_pton");
		return EXIT_FAILURE;
	}

	// Connect to the server
	int connect_result = connect(sockfd, (struct sockaddr*) &servaddr, servlen);
	if (connect_result < 0) {
		perror("connect");
		return EXIT_FAILURE;
	}

	// Send the line
	int writen_result = exso_writen(sockfd, line, strlen(line));
	
	// Check for writen errors
	if (writen_result < 0) {
		perror("exso_writen");
		return EXIT_FAILURE;
	}

	// Read server reply
	memset(line, 0, MAXLINE);
	int n = exso_readln(sockfd, line, MAXLINE);
	
	// Check for readln errors
	if (n < 0) {
		perror("exso_readln");
		return EXIT_FAILURE;
	}

	// Check for server errors
	if (n == 0) {
		printf("Server terminated prematurely\n");
		return EXIT_FAILURE;
	}

	fputs(line, stdout);

	int close_result = close(sockfd);
	if (close_result < 0) {
		perror("close");
		return EXIT_FAILURE;
	}
	
	return EXIT_SUCCESS;
}
