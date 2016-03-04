#include "../basic.h"

#define TESTS_TXT "tests.txt"
#define READ_MODE "r"

void zombie_handler (int signum);

int server_examinator (int sockfd);

int count_students (int tests_min, int threshold);

/*
 * This server gets two integers from a connected client,
 * THRESHOLD and NUMBER, then reads the results within the
 * file "tests.txt" and sends to the client the number of
 * students which have taken at least NUMBER tests and
 * obtained a total score above THRESHOLD.
 */
int main (int argc, char** argv)
{
	// Check arguments
	if (argc != 2) {
		printf("Usage: ./exaser <PORT>\n");
		return EXIT_FAILURE;
	}

	// Handle zombies
	if (signal(SIGCHLD, zombie_handler) == SIG_ERR) {
		perror("signal");
		return EXIT_FAILURE;
	}

	// Create a socket
	int listenfd = socket(AF_INET6, SOCK_STREAM, 0);
	if (listenfd < 0) {
		perror("socket");
		return EXIT_FAILURE;
	}

	// Initialize a server sockaddr structure
	struct sockaddr_in6 servaddr;
	socklen_t servlen = sizeof(servaddr);
	bzero(&servaddr, servlen);
	servaddr.sin6_family = AF_INET6;
	servaddr.sin6_addr = in6addr_any;
	servaddr.sin6_port = htons(atoi(argv[1]));

	// Bind the socket
	int bind_result = bind(listenfd, (struct sockaddr*) &servaddr, servlen);
	if (bind_result < 0) {
		perror("bind");
		return EXIT_FAILURE;
	}

	// Convert the socket to a listening socket
	int listen_result = listen(listenfd, BACKLOG);
	if (listen_result < 0) {
		perror("listen");
		return EXIT_FAILURE;
	}

	// Wait for clients
	int connfd;
	struct sockaddr_in6 cliaddr;
	socklen_t clilen = sizeof(cliaddr);
	pid_t childpid;
	
	while (TRUE) {
		connfd = accept(listenfd, (struct sockaddr*) &cliaddr, &clilen);

		// Handle accept errors
		if (connfd < 0) {
			if (errno == EINTR) {
				/* The system call was interrupted by a signal that was
				 * caught before a valid connection arrived; so continue */
				continue;
			} else {
				perror("accept");
				return EXIT_FAILURE;
			}
		}

		// Create a child process to serve a new client
		childpid = fork();
		if (childpid == 0) {
			int close_result = close(listenfd);
			if (close_result < 0) {
				perror("close");
				return EXIT_FAILURE;
			}
			int server_result = server_examinator(connfd);
			return server_result;
		}

		// While the parent close the connected socket
		int close_result = close(connfd);
		if (close_result < 0) {
			perror("close");
			return EXIT_FAILURE;
		}
	}

	return EXIT_SUCCESS;
}

void zombie_handler (int signum)
{
	pid_t pid;
	int stat;

	// Kill zombies
	while (TRUE) {
		pid = waitpid(WAIT_ANY, &stat, WNOHANG);
		if (pid > 0) {
			printf("%d: Child %d terminated with exit status %d\n", signum, pid, stat);
		}
		else return;
	}
}

int server_examinator (int sockfd)
{
	char line[MAXLINE];
	memset(line, 0, MAXLINE);
	int n;

	// Read from client
	n = exso_readln(sockfd, line, MAXLINE);

	// Check for readln errors
	if (n < 0) {
		perror("exso_readln");
		return EXIT_FAILURE;
	}
   	else if (n == 0) {
		printf("Client disconnected\n");
		return EXIT_SUCCESS;
	}

	// Get tests_min
	char* tests_min_string = strtok(line, " ");
	if (tests_min_string == NULL) {
		perror("strtok error while reading tests_min");
		return EXIT_FAILURE;
	}
	printf("Received tests_min %s\n", tests_min_string);
	int tests_min = atoi(tests_min_string);
	
	// Get threshold
	char* threshold_string = strtok(NULL, " ");
	if (threshold_string == NULL) {
		perror("strtok error while reading threshold");
		return EXIT_FAILURE;
	}
	printf("Received threshold_string %s\n", threshold_string);
	int threshold = atoi(threshold_string);

	// Find students_count
	int students_count = count_students(tests_min, threshold);
	
	// Prepare reply
	memset(line, 0, MAXLINE);
	sprintf(line, "%d\n", students_count);

	// Send reply
	n = exso_writen(sockfd, line, n);
	if (n < 0) {
		perror("exso_writen");
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

int count_students(int tests_min, int threshold) {
	int students_count = 0;

	// Open tests file
	FILE* tests_file = fopen(TESTS_TXT, READ_MODE);	

	// Check for open errors
	if (tests_file == NULL) {
		perror("fopen");
		return EXIT_FAILURE;
	}

	char line[MAXLINE];

	// Read line by line
	while (TRUE) {
		memset(line, 0, MAXLINE);
		fgets(line, MAXLINE, tests_file);
		
		// Check for end of file or read errors
		if (feof(tests_file)) {
			break;
		} else if (ferror(tests_file)) {
			perror("fgets");
			return EXIT_FAILURE;
		}

		// Read id
		char* id_string = strtok(line, " ");
		if (id_string == NULL) {
			perror("strtok error while reading id");
			return EXIT_FAILURE;
		}

		// Read first name
		char* first_name = strtok(NULL, " ");
		if (first_name == NULL) {
			perror("strtok error while reading first name");
			return EXIT_FAILURE;
		}

		// Read last name
		char* last_name = strtok(NULL, " ");
		if (last_name == NULL) {
			perror("strtok error while reading last name");
		}
		
		int vote_sum = 0;
		int vote_count = 0;

		// Compute the vote sum and the number of tests
		while (TRUE) {
			char* vote_string = strtok(NULL, " ");
			if (vote_string) {
				vote_sum += atoi(vote_string);
				++vote_count;
			} else break;
		}

		// Compute average
		if (vote_count >= tests_min) {
			int average = vote_sum / vote_count;
			printf("Checking student %s: %s %s - %d tests - %d average\n",
					id_string, first_name, last_name, vote_count, average);

			if (average >= threshold) {
				students_count++;					
			}
		}
	}
	
	// Close the file
	int fclose_result = fclose(tests_file);
	if (fclose_result == EOF) {
		perror("fclose");
		return EXIT_FAILURE;
	}

	return students_count;
}
