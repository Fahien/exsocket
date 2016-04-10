#include "../basic.h"

#define WELCOME_MESSAGE "Welcome to ecommerce!\n"
#define PRODUCT_PURCHASED_MESSAGE "Product purchased!\n"
#define PRODUCT_NOT_FOUND_MESSAGE "Product not found!\n"
#define ERROR_MESSAGE "Command not valid\n"
#define END_MESSAGE ".\n"

#define PRODUCTS_FILENAME "products.txt"

#define BYE_COMMAND "BYE"
#define SEARCH_COMMAND "SEARCH"
#define BUY_COMMAND "BUY"

typedef void (*sig_t)(int);

/*
 * Zombie handler
 */
void zombie_handler(int signo)
{
    printf("Captured signal %d\n", signo);
    pid_t wait_result;
    int status;
    while (true) {
        // Wait any process
        wait_result = waitpid(WAIT_ANY, &status, WNOHANG);
        // Break if no children has terminated
        if (wait_result <= 0) break;
        // Print informations
        printf("Children %d terminated with status %d\n", wait_result, status);
    }
}

/*
 * Handle ERROR command
 */
int error_handler(int connfd) {
    int writen_result = exso_writen(connfd, ERROR_MESSAGE, strlen(ERROR_MESSAGE));
    if (writen_result < 0) {
        perror("Could not send error message");
        return EXIT_FAILURE;
    }
    writen_result = exso_writen(connfd, END_MESSAGE, strlen(END_MESSAGE));
    if (writen_result < 0) {
        perror("Could not send end message");
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

/*
 * Search for category
 */
int search_handler(int connfd, char* category) {
    // Open products.txt in read mode
    FILE* products = fopen(PRODUCTS_FILENAME, "r");
    // Check fopen errors
    if (products == NULL) {
        perror("Could not open " PRODUCTS_FILENAME);
        return EXIT_FAILURE;
    }

    // Loop variables
    int pid;
    char pname[MAXLINE];
    char pcategory[MAXLINE];
    char buffer[MAXLINE];
    int write_result;

    while (true) {
        // Reset variables
        memset(pname, 0, MAXLINE);
        memset(pcategory, 0, MAXLINE);
        memset(buffer, 0, MAXLINE);
        // Read a product line
        int scanf_result = fscanf(products, "%d %s %s", &pid, pname, pcategory);
        // Check for fscanf error
        if (ferror(products)) {
            perror("Could not read a product line");
            break;
        }
        // Check for fscanf error
        if (scanf_result != 3) {
            write_result = exso_writen(connfd, END_MESSAGE, strlen(END_MESSAGE));
            break;
        }

        // Compare category
        bool to_send = memcmp(category, pcategory, strlen(category)) == 0;
        to_send = to_send && strlen(category) == strlen(pcategory);
        if (to_send) {
            // Send a product line
            sprintf(buffer, "%d %s %s\n", pid, pname, pcategory);
            write_result = exso_writen(connfd, buffer, strlen(buffer));
            // Check for writen error
            if (write_result < 0) {
                perror("Could not write on connected socket");
                break;
            }
        }
    }

    // Close file
    int close_result = fclose(products);
    if (close_result == EOF) {
        perror("Could not close " PRODUCTS_FILENAME);
    }
    // Return any error
    return write_result && close_result;
}

/*
 * Buy a product
 */
int buy_handler(int connfd, char* operator)
{
    // Open products file
    FILE* products = fopen(PRODUCTS_FILENAME, "r");
    // Check fopen errors
    if (products == NULL) {
        perror("Could not open " PRODUCTS_FILENAME);
        return EXIT_FAILURE;
    }

    int id = atoi(operator);
    // Loop variables
    int pid;
    char pname[MAXLINE];
    char pcategory[MAXLINE];
    int write_result;
    bool product_found = false;

    while (true) {
        // Clear variables
        memset(pname, 0, MAXLINE);
        memset(pcategory, 0, MAXLINE);

        int scanf_result = fscanf(products, "%d %s %s", &pid, pname, pcategory);
        // Check for fscanf error
        if (ferror(products)) {
            perror("Could not read a product line");
            break;
        }
        // Check for EOF
        if (scanf_result == EOF) break;
        // Check for fscanf error
        if (scanf_result != 3) {
            write_result = exso_writen(connfd, END_MESSAGE, strlen(END_MESSAGE));
            break;
        }
        
        // Compare id
        product_found = (id == pid);
        if (product_found) {
            // Send the product line
            write_result = exso_writen(connfd, PRODUCT_PURCHASED_MESSAGE, strlen(PRODUCT_PURCHASED_MESSAGE));
            // Check for writen error
            if (write_result < 0) {
                perror("Could not write on connected socket");
                break;
            }
            write_result = exso_writen(connfd, END_MESSAGE, strlen(END_MESSAGE));
            break;
        }
    }

    // Send product not found message
    if (!product_found) {
        write_result = exso_writen(connfd, PRODUCT_NOT_FOUND_MESSAGE, strlen(PRODUCT_NOT_FOUND_MESSAGE));
        // Check for writen error
        if (write_result < 0) {
            perror("Could not write on connected socket");
        } else {
            write_result = exso_writen(connfd, END_MESSAGE, strlen(END_MESSAGE));
        }
    }

    // Close products file
    int close_result = fclose(products);
    if (close_result == EOF) {
        perror("Could not close " PRODUCTS_FILENAME);
    }
    // Return any error
    return write_result && close_result;
}


/*
 * Client handler
 */
int client_handler(int connfd)
{
    // Welcome message
    char buffer[MAXLINE];
    memset(buffer, 0, MAXLINE);
    memcpy(buffer, WELCOME_MESSAGE, strlen(WELCOME_MESSAGE));
    
    // Send welcome message
    int writen_result = exso_writen(connfd, buffer, strlen(buffer));
    // Check writen error
    if (writen_result < 0) {
        perror("Could not send welcome message");
        return EXIT_FAILURE;
    }

    // Logic variables
    char operation[MAXLINE];
    memset(operation, 0, MAXLINE);
    char parameter[MAXLINE];
    memset(operation, 0, MAXLINE);

    while (true) {
        // Reset buffer
        memset(buffer, 0, MAXLINE);
        // Wait for client command
        int read_result = exso_readln(connfd, buffer, MAXLINE);
        // Check for read error
        if (read_result < 0) {
            perror("Could not read from client");
            return EXIT_FAILURE;
        }

        // Check for disconnection
        if (read_result == 0) {
            printf("Client disconnected\n");
            return EXIT_SUCCESS;
        }

        // Handle BYE command
        bool bye_command = memcmp(buffer, BYE_COMMAND, strlen(BYE_COMMAND)) == 0;
        if (bye_command) {
            return EXIT_SUCCESS;
        }

        // Read Operation and Parameter
        int scan_result = sscanf(buffer, "%s %s", operation, parameter);
        if (scan_result != 2) {
            fprintf(stderr, "Could not read operation and/or parameter\n");
            // Handle error
            int error_result = error_handler(connfd);
            if (error_result < 0) {
                fprintf(stderr, "Could not handle error");
            }
            continue;
        }
        printf("Handling operation %s and parameter %s\n", operation, parameter);

        // Handle SEARCH command
        bool search_command = memcmp(operation, SEARCH_COMMAND, strlen(SEARCH_COMMAND)) == 0;
        if (search_command) {
            int search_result = search_handler(connfd, parameter);
            // Check search error
            if (search_result < 0) {
                fprintf(stderr, "Could not handle search command\n");
                return EXIT_FAILURE;
            }
            continue;
        }

        // Handle BUY command
        bool buy_command = memcmp(operation, BUY_COMMAND, strlen(BUY_COMMAND)) == 0;
        if (buy_command) {
            int buy_result = buy_handler(connfd, parameter);
            // Check buy error
            if (buy_result < 0) {
                fprintf(stderr, "Could not handle buy command\n");
                return EXIT_FAILURE;
            }
            continue;
        }

        // Handle error
        int error_result = error_handler(connfd);
        if (error_result < 0) {
            fprintf(stderr, "Could not handle error");
        }
    }
}

/*
 * Main function
 */
int main (int argc, char** argv)
{
    // Check arguments
    if (argc != 2) {
        fprintf(stderr, "Usage: ./ecommerceser <PORT>\n");
        return EXIT_FAILURE;
    }

    // Create sockaddr structure
    struct sockaddr_in servaddr;
    socklen_t servlen = sizeof servaddr;
    memset(&servaddr, 0, servlen);
    // Set address family
    servaddr.sin_family = AF_INET;
    // Set port number
    servaddr.sin_port = htons(atoi(argv[1]));
    // Set in address
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);

    // Create socket
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    // Check for socket error
    if (sockfd < 0) {
        perror("Could not create a socket");
        return EXIT_FAILURE;
    }

    // Bind socket
    int bind_result = bind(sockfd, (struct sockaddr*) &servaddr, servlen);
    // Check bind error
    if (bind_result < 0) {
        perror("Could not bind socket");
        return EXIT_FAILURE;
    }

    // Convert to a listening socket
    int listen_result = listen(sockfd, BACKLOG);
    // Check listen error
    if (listen_result < 0) {
        perror("Could not convert to a listening socket");
        return EXIT_FAILURE;
    }

    // Create client sockaddr structure
    struct sockaddr_in cliaddr;
    socklen_t clilen = sizeof cliaddr;
    memset(&cliaddr, 0, clilen);
    char cliname[MAXLINE];
    memset(cliname, 0, MAXLINE);

    while (true) {
        // Wait for and accept incoming connection
        int connfd = accept(sockfd, (struct sockaddr*) &cliaddr, &clilen);
        // Check accept error
        if (connfd < 0) {
            if (errno == EINTR) continue;
            perror("Could not accept incoming connection");
            continue;
        }

        // Print client informations
        inet_ntop(AF_INET, &servaddr, cliname, clilen);
        printf("Client connected %s\n", cliname);

        // Register zombie handler
        sig_t signal_result = signal(SIGCHLD, zombie_handler);
        // Check signal error
        if (signal_result == SIG_ERR) {
            perror("Could not register zombie handler");
            return EXIT_FAILURE;
        }

        // Create a child to handle the client
        pid_t child = fork();
        // Check fork error
        if (child < 0) {
            perror("Could not create a child");
        }

        // Child code
        if (child == 0) {
            int client_result = client_handler(connfd);
            // Check client handler error
            if (client_result < 0) {
                perror("Could not handle client");
            }
            // Close socket
            int close_result = close(connfd);
            // Check close error
            if (close_result < 0) {
                perror("Could not close connection");
            }
            // Return any error
            return client_result && close_result;
        }
        else {
            // Parent close connected socket
            int close_result = close(connfd);
            // Check close error
            if (close_result < 0) {
                perror("Could not close connected socket");
                return EXIT_FAILURE;
            }
        }
    }

    return EXIT_SUCCESS;
}
