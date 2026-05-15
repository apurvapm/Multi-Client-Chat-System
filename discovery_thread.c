#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>

#define PORT 9000
#define MAX_USERS 50
#define BUFFER 1024

typedef struct
{
    char username[32];
    char password[32];
} user_t;

user_t users[MAX_USERS];
int user_count = 0;

pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

void *handle_client(void *arg)
{
    int sock = *(int *)arg;
    free(arg);

    char buffer[BUFFER];

    while (1)
    {
        int n = recv(sock, buffer, BUFFER - 1, 0);

        if (n <= 0)
            break;

        buffer[n] = '\0';

        char cmd[32];
        sscanf(buffer, "%s", cmd);

        if (strcmp(cmd, "REGISTER") == 0)
        {
            char user[32], pass[32];

            sscanf(buffer, "REGISTER %s %s", user, pass);

            pthread_mutex_lock(&lock);

            strcpy(users[user_count].username, user);
            strcpy(users[user_count].password, pass);
            user_count++;

            pthread_mutex_unlock(&lock);

            send(sock, "REGISTER_OK\n", 12, 0);
        }

        else if (strcmp(cmd, "GET_SERVER") == 0)
        {
            send(sock, "CHAT_SERVER 127.0.0.1 10000\n", 27, 0);
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

    printf("Discovery Thread Server running on port %d\n", PORT);

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
    return 0;
}