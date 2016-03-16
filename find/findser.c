#include "../basic.h"

/*
 * Server that reads from a client a <FILE> and a <STRING>
 * then searches within the file the line in which the string
 * appears more times and returns the line number and number
 * of occurrences of the string in that row
 */
int checked_close(int sockfd);

int server_find(int connfd);
int file_find(const char *file, const char *token, int *occurrencies, int *line);

typedef void (*sighandler_t)(int);
static void zombie_handler(int signum);

int main(int argc, char** argv)
{
	// Check arguments
	if (argc != 2) {
		fprintf(stderr, "Usage: ./findser <PORT>\n");
		return EXIT_FAILURE;
	}
	

	// Create a socket
	int listenfd = socket(AF_INET6, SOCK_STREAM, 0);
	// Check for socket error
	if (listenfd < 0) {
		perror("socket");
		return EXIT_FAILURE;
	}

	// Create server sockaddr structure
	struct sockaddr_in6 servaddr;
	socklen_t servlen = sizeof servaddr;
	memset(&servaddr, 0, servlen);
	servaddr.sin6_family = AF_INET6;
	servaddr.sin6_addr = in6addr_any;
	servaddr.sin6_port = htons(atoi(argv[1]));

	// Bind the socket
	int bind_result = bind(listenfd, (struct sockaddr*) &servaddr, servlen);
	// Check for bind errors
	if (bind_result < 0) {
		perror("bind");
		return EXIT_FAILURE;
	}

	// Convert the socket to a listening socket
	int listen_result = listen(listenfd, BACKLOG);
	// Check for listen errors
	if (listen_result < 0) {
		perror("listen");
		return EXIT_FAILURE;
	}

	// Create client sockaddr structure
	struct sockaddr_in6 cliaddr;
	socklen_t clilen = sizeof cliaddr;
	memset(&cliaddr, 0, clilen);

	// Accept incoming connections
	while (true) {
		int connfd = accept(listenfd, (struct sockaddr*) &cliaddr, &clilen);
		// Check for accept errors
		if (connfd < 0) {
			if (errno == EINTR) {
				// Interrupted by a signal caught before a valid connection arrived
				continue;
			} else {
				perror("accept");
				return EXIT_FAILURE;
			}
		}

		// Register zombie handler
		sighandler_t signal_result = signal(SIGCHLD, zombie_handler);
		// Check for sigaction errors
		if (signal_result == SIG_ERR) {
			perror("signal");
			return EXIT_FAILURE;
		}

		// Fork this process
		int fork_result = fork();
		// Check for fork errors
		if (fork_result < 0) {
			perror("fork");
			checked_close(connfd);
			return EXIT_FAILURE;
		}

		// The child handles the request
		if (fork_result == 0) {
			// Close listening socket
			int listen_close_result = checked_close(listenfd);
			// Compute application logic
			int find_result = server_find(connfd);
			// Close connected socket
			int conn_close_result = checked_close(connfd);
			// Return any error
			return listen_close_result && find_result && conn_close_result;
		} else {
			// The parent closes connected socket
			int close_result = checked_close(connfd);
			// Check for close errors
			if (close_result < 0) {
				// Close the listeninf socket
				checked_close(listenfd);
				return EXIT_FAILURE;
			}
		}
	}

	return EXIT_SUCCESS;
}

int checked_close(int sockfd)
{
	// Close descriptor
	int close_result = close(sockfd);
	// Check for close errors
	if (close_result < 0) {
		perror("close");
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}

int server_find(int connfd)
{
	// Initialize a line buffer
	char buffer[MAXLINE + 1];
	memset(buffer, 0, MAXLINE);
	// Read from connected client
	int n = exso_readln(connfd, buffer, MAXLINE);
	// Check for disconnection
	if (n == 0) {
		fprintf(stderr, "Client disconnected\n");
		return EXIT_FAILURE;
	}
	// Check for exso_readln error
	if (n < 0) {
		perror("exso_readln");
		return EXIT_FAILURE;
	}
	
	// Initialize file buffer
	char file[MAXLINE + 1];
	memset(file, 0, MAXLINE);
	// Initialize token buffer
	char token[MAXLINE + 1];
	memset(token, 0, MAXLINE);
	// Read file name and the token to find from the line buffer
	sscanf(buffer, "%s %s", file, token);

	// Reset buffer for later use
	memset(buffer, 0, MAXLINE);

	int line = 0;
	int occurrencies = 0;
	// Find line and occurrencies
	int find_result = file_find(file, token, &line, &occurrencies);
	// Check for find errors
	if (find_result < 0) {
		sprintf(buffer, "Server error\n");
	}
	else {
		sprintf(buffer, "Found %d occurrencies in line %d\n", line, occurrencies);
	}

	// Reply to connected client
	int writen_result = exso_writen(connfd, buffer, strlen(buffer));
	// Check for writen error
	if (writen_result < 0) {
		perror("exso_writen");
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

int file_find(const char *file, const char *token, int *occurrencies, int *line)
{
	FILE *fp = fopen(file, "r");

	// Check for open errors
	if (!fp) {
		perror("fopen");
		return EXIT_FAILURE;
	}

	// Initialize variables
	*line = 0;
	*occurrencies = 0;

	int line_ = 0;
	int occurrencies_ = 0;
	char buffer[MAXLINE];
	memset(buffer, 0, MAXLINE);

	char *fgets_result;
	char *substring;

	while (true) {
		// Get a line from file
		fgets_result = fgets(buffer, MAXLINE, fp);
		// Check for fgets errors
		if (!fgets_result) break;
		
		++line_;
		substring = buffer;
		while (true) {
			// Find occurrencies of token in current line
			substring = strstr(substring, token);
			// Break if the substring is not found
			if (!substring) break;
			else {
				++occurrencies_;
				++substring;
			}
		}

		// Update return values
		if (occurrencies_ > *occurrencies) {
			*occurrencies = occurrencies_;
			*line = line_;
		}
		occurrencies_ = 0;
	}
	
	// Close file
	int fclose_result = fclose(fp);
	if (fclose_result == EOF) {
		perror("fclose");
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

void zombie_handler(int signum)
{
	printf("Sig %d captured\n", signum);
	int status;
	int waitpid_result = 0;
	while (true) {
		// Wait for any child
		waitpid_result = waitpid(WAIT_ANY, &status, WNOHANG);
		// Check for no more child
		if (waitpid_result == 0) {
			printf("\tChild working\n");
			return;
		}
		// Check for waitpid errors
		if (waitpid_result < 0) {
			perror("\twaitpid");
			return;
		}
		printf("\tChild process %d terminated with exit status %d\n", waitpid_result, status);
	}
}
