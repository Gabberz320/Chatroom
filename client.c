#include "csapp.h"

#define BUFFSIZE 1024
//Global variables 
int socket_fd;  //file descriptor to communicate with the server
char username[50]; //username

//Function prototypes
void *receiver(void *arg);
void sendMessage(const char *message);

    

int main(int argc, char *argv[])
{
   
    char *host, *port;
    
    char buffer[MAXLINE]; //MAXLINE = 8192 defined in csapp.h
    size_t n;

    if (argc != 3)
    {
        fprintf(stderr, "usage: %s <host> <port>\n", argv[0]);
    	exit(0);
    }

    host = argv[1];
    port = argv[2];

    socket_fd = Open_clientfd(host, port); //wrapper function that calls getadderinfo, socket and connect functions for client side

    //Get the welcome message from the server
    n = read(socket_fd, buffer, sizeof(buffer));
    if (n < 0) {
        fprintf(stderr, "failed to receive welcome message\n");
        exit(1);
    }

    buffer[n] = '\0'; // null-terminate the string
    printf("%s\n", buffer); // print the welcome message

    // Get username from the user
    printf("Enter your username: ");
    Fgets(username, sizeof(username), stdin);
    username[strcspn(username, "\n")] = '\0'; // remove newline character


    // Send username to the server
    n = write(socket_fd, username, strlen(username));
    if (n < 0) {
        fprintf(stderr, "failed to send username\n");
        exit(1);
    }
    
    //Parallel thread to receive messages
    pthread_t tid;
    pthread_create(&tid, NULL, receiver, NULL);

    // Main loop to send messages to the server
    while (1){
        printf("> ");
        fflush(stdout); // flush stdout to ensure prompt is displayed
        Fgets(buffer, sizeof(buffer), stdin);
        buffer[strcspn(buffer, "\n")] = '\0'; // remove newline character
        if (strcmp(buffer, "exit") == 0) {
            sendMessage("exit");
            break; // exit the loop
        }

        sendMessage(buffer);  //Call send message to send to server
        
    }
    close(socket_fd); // close the socket

    return 0;
}

// Function to receive messages from the server
void *receiver(void *arg)
{
    char buffer[BUFFSIZE];
    size_t n;
    // Initialize the file descriptor set
    fd_set readfds;
    struct timeval timeout; // timeout for select function
    
    while (1)
    {
        FD_ZERO(&readfds); //Clear readfds
        FD_SET(socket_fd, &readfds);  //Add fd to readfds

        timeout.tv_sec = 1; // set timeout to 1 second
        timeout.tv_usec = 0; // set microseconds to 0

        //call select function to check if there is any data to read
        int activity = select(socket_fd + 1, &readfds, NULL, NULL, &timeout);
        if (activity < 0)
        {
            printf("select error\n");
            continue;
        }
        //if the socket_fd was set, read the data into buffer
        if(FD_ISSET(socket_fd, &readfds))
        {
            n = read(socket_fd, buffer, sizeof(buffer));
            if (n <= 0)
            {
                printf("Disconnected from server\n");
                exit(0);
            }
            buffer[n] = '\0'; // null-terminate the string
            printf("\n%s\n> ", buffer); // print the message
            fflush(stdout); // flush stdout to ensure prompt is displayed
        }
    }
}
// Function to send message to the server
void sendMessage(const char *message)
{
    size_t n = write(socket_fd, message, strlen(message));
    if (n < 0)
    {
        fprintf(stderr, "failed to send message\n");
        exit(1);
    }
}
