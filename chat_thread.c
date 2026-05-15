#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>

#define PORT 10000
#define MAX_CLIENTS 50
#define BUFFER 1024

typedef struct
{
    int socket;
    char username[32];
} client_t;

client_t clients[MAX_CLIENTS];
int client_count = 0;

pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

void remove_client(int sock)
{
    pthread_mutex_lock(&lock);

    for (int i = 0; i < client_count; i++)
    {
        if (clients[i].socket == sock)
        {
            printf("User %s disconnected\n", clients[i].username);

            for (int j = i; j < client_count - 1; j++)
                clients[j] = clients[j + 1];

            client_count--;
            break;
        }
    }

    pthread_mutex_unlock(&lock);
}

void broadcast_message(char *sender, char *msg)
{
    char buffer[BUFFER];

    sprintf(buffer, "MESSAGE %s: %s\n", sender, msg);

    pthread_mutex_lock(&lock);

    for (int i = 0; i < client_count; i++)
    {
        send(clients[i].socket, buffer, strlen(buffer), 0);
    }

    pthread_mutex_unlock(&lock);
}

void private_message(char *sender, char *receiver, char *msg)
{
    char buffer[BUFFER];

    sprintf(buffer, "PRIVATE %s: %s\n", sender, msg);

    pthread_mutex_lock(&lock);

    for (int i = 0; i < client_count; i++)
    {
        if (strcmp(clients[i].username, receiver) == 0)
        {
            send(clients[i].socket, buffer, strlen(buffer), 0);
            break;
        }
    }

    pthread_mutex_unlock(&lock);
}

void send_user_list(int sock)
{
    char buffer[BUFFER] = "USERS ";

    pthread_mutex_lock(&lock);

    for (int i = 0; i < client_count; i++)
    {
        strcat(buffer, clients[i].username);
        strcat(buffer, " ");
    }

    pthread_mutex_unlock(&lock);

    strcat(buffer, "\n");

    send(sock, buffer, strlen(buffer), 0);
}

void *handle_client(void *arg)
{
    int sock = *(int *)arg;
    free(arg);

    char buffer[BUFFER];
    char username[32] = "";

    while (1)
    {
        int n = recv(sock, buffer, BUFFER - 1, 0);

        if (n <= 0)
        {
            remove_client(sock);
            break;
        }

        buffer[n] = '\0';

        char cmd[32];
        sscanf(buffer, "%s", cmd);

        if (strcmp(cmd, "LOGIN") == 0)
        {
            char pass[32];

            sscanf(buffer, "LOGIN %s %s", username, pass);

            pthread_mutex_lock(&lock);

            strcpy(clients[client_count].username, username);
            clients[client_count].socket = sock;
            client_count++;

            pthread_mutex_unlock(&lock);

            send(sock, "LOGIN_OK\n", 9, 0);

            printf("%s logged in\n", username);
        }

        else if (strcmp(cmd, "BROADCAST") == 0)
        {
            char msg[900];
            sscanf(buffer, "BROADCAST %[^\n]", msg);

            broadcast_message(username, msg);
        }

        else if (strcmp(cmd, "PRIVATE") == 0)
        {
            char receiver[32], msg[900];

            sscanf(buffer, "PRIVATE %s %[^\n]", receiver, msg);

            private_message(username, receiver, msg);
        }

        else if (strcmp(cmd, "LIST") == 0)
        {
            send_user_list(sock);
        }

        else if (strcmp(cmd, "LOGOUT") == 0)
        {
            remove_client(sock);
            close(sock);
            return NULL;
        }
    }

    close(sock);
    return NULL;
}

int main()
{
    int server_socket, client_socket;
    struct sockaddr_in server_addr;

    server_socket = socket(AF_INET, SOCK_STREAM, 0);

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr));
    listen(server_socket, 10);

    printf("Chat Thread Server running on port %d\n", PORT);

    while (1)
    {
        client_socket = accept(server_socket, NULL, NULL);

        pthread_t tid;

        int *pclient = malloc(sizeof(int));
        *pclient = client_socket;

        pthread_create(&tid, NULL, handle_client, pclient);
        pthread_detach(tid);
    }

    close(server_socket);
}