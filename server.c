#include "csapp.h"

//max size of clients and length of the string
#define MAX_CLIENT 10   
#define MAXSIZE 1024 

// Define struct to hold client information
typedef struct {
    int connfd; // file descriptor for the client connection
    char username[50]; // username of the client
} client_t;

//Global Variables
client_t * clients[MAX_CLIENT]; // array of client_t structs to hold client information
pthread_mutex_t clients_mutex; // mutex for synchronizing access to clients array

//function prototypes
void *handle_client(void *arg);
void broadcast(char *message, int sender_fd);
void add_client(client_t *client);
void remove_client(int connfd);



int main(int argc, char *argv[])
{
    int listenfd, connfd; 
    int *connfdp;
     //file descriptor to communicate with the client
    socklen_t clientlen;
    struct sockaddr_storage clientaddr;  /* Enough space for any address */
    
    

    if (argc != 2) {
	fprintf(stderr, "usage: %s <port>\n", argv[0]);
	exit(0);
    }

    listenfd = Open_listenfd(argv[1]); //wrapper function that calls getadderinfo, socket, bind, and listen functions in the server side

    //Server runs in the infinite loop.
    //To stop the server process, it needs to be killed using the Ctrl+C key.
    while (1) {
    	clientlen = sizeof(struct sockaddr_storage);

        //Accept client connection
        connfd = Accept(listenfd, (SA *)&clientaddr, &clientlen);
        connfdp = Malloc(sizeof(int));
        *connfdp = connfd;

        pthread_t client_thread;
        pthread_create(&client_thread,NULL,handle_client,(void *)connfdp);            
    }
    return 0;
}
//Thread for concurrency
void *handle_client(void *arg){
    int connfd = *((int *)arg); // retrieve client socket address
    free(arg); //free memory
    char buffer[MAXSIZE];
    size_t n;

    //send welcome message
    char welcome_message[MAXSIZE] = "Welcome to the chatroom.\nPlease Remember to always be kind and curtious to other members.\n";
    n = write(connfd, welcome_message, strlen(welcome_message));
    if (n < 0) {
        fprintf(stderr, "failed to write\n");
        exit(1);
    }

    //revceive the username
    if((n = read(connfd,buffer, sizeof(buffer)) <= 0)){
        close(connfd);
        return NULL;
    }
    
    char username[50];
    strcpy(username, buffer); //copy the username to the username variable

    //create a new client struct and add it to the clients array
    client_t *client = malloc(sizeof(client_t));
    client->connfd = connfd; //sore the client connfd
    strcpy(client->username, username); //store the client username
    add_client(client); //add the client to the clients array

    // Inform the users the a new user has joined
    char join_message[MAXLINE];
    sprintf(join_message, "%s has joined the chatroom.\n", username); //prepare the message
    broadcast(join_message, connfd); //broadcast the message to all clients

    // Listen for other clients messages
    while ((n = read(connfd, buffer, sizeof(buffer))) > 0) {
        buffer[n] = '\0'; // Null terminate the received message
        if(strcmp(buffer, "exit") == 0) {
            break; // exit the loop if the client sends "exit"
        }
        // Broadcast the message to all clients
        char message[MAXLINE];
        sprintf(message, "%s: %s", username, buffer); //prepare the message
        broadcast(message, connfd); //broadcast the message to all clients
        
    }
    

    // Client disconnected
    remove_client(connfd); //remove the client from the clients array

    // Inform the users that the client has left
    char leave_message[MAXLINE];
    sprintf(leave_message, "%s has left the chatroom.\n", username); //prepare the message
    broadcast(leave_message, connfd); //broadcast the message to all clients

    close(connfd); //close the client connection
    return NULL;

}
void broadcast(char *message, int sender_fd) {
    size_t n;
    pthread_mutex_lock(&clients_mutex); //lock the mutex
    for (int i = 0; i < MAX_CLIENT; i++) {
        if (clients[i] != NULL && clients[i]->connfd != sender_fd) {
            n = write(clients[i]->connfd, message, strlen(message)); //send the message to all clients except the sender
            if (n < 0) {
                fprintf(stderr, "failed to write\n");
                exit(1);
            }
        }
    }
    pthread_mutex_unlock(&clients_mutex); //unlock the mutex
}
void add_client(client_t *client) {
    pthread_mutex_lock(&clients_mutex); //lock the mutex
    for (int i = 0; i < MAX_CLIENT; i++) {
        if (clients[i] == NULL) {
            clients[i] = client; //add the client to the clients array
            break;
        }
    }
    pthread_mutex_unlock(&clients_mutex); //unlock the mutex
}
void remove_client(int connfd) {
    pthread_mutex_lock(&clients_mutex); //lock the mutex
    for (int i = 0; i < MAX_CLIENT; i++) {
        if (clients[i] != NULL && clients[i]->connfd == connfd) {
            free(clients[i]); //free the memory allocated for the client
            clients[i] = NULL; //remove the client from the clients array
            break;
        }
    }
    pthread_mutex_unlock(&clients_mutex); //unlock the mutex
}
