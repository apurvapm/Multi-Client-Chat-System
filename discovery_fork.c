// discovery_fork.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/wait.h>

#define PORT 9000
#define MAX_USERS 100
#define BUF 1024

typedef struct {
    char username[50];
    char password[50];
    char ip[INET_ADDRSTRLEN];
    int port;
    int active;
} user;

user users[MAX_USERS];
int user_count = 0;

void send_user_list(int sock) {
    char buf[BUF];

    for(int i=0;i<user_count;i++){
        if(users[i].active){
            sprintf(buf,"USER %s %s %d\n",
                    users[i].username,
                    users[i].ip,
                    users[i].port);
            send(sock,buf,strlen(buf),0);
        }
    }

    send(sock,"END\n",4,0);
}

void handle_client(int sock, struct sockaddr_in client){

    char buf[BUF];
    char cmd[50],usern[50],pass[50];
    int port;

    while(1){

        int n = recv(sock,buf,BUF-1,0);
        if(n<=0) break;
        buf[n]=0;

        sscanf(buf,"%s",cmd);

        if(strcmp(cmd,"REGISTER")==0){

            sscanf(buf,"%s %s %s %d",cmd,usern,pass,&port);

            strcpy(users[user_count].username,usern);
            strcpy(users[user_count].password,pass);
            strcpy(users[user_count].ip,inet_ntoa(client.sin_addr));
            users[user_count].port=port;
            users[user_count].active=1;

            user_count++;

            send(sock,"OK\n",3,0);
        }

        else if(strcmp(cmd,"LOGIN")==0){

            sscanf(buf,"%s %s %s",cmd,usern,pass);

            int found=0;

            for(int i=0;i<user_count;i++){

                if(strcmp(users[i].username,usern)==0 &&
                   strcmp(users[i].password,pass)==0){

                    users[i].active=1;
                    found=1;
                    break;
                }
            }

            if(found) send(sock,"OK\n",3,0);
            else send(sock,"FAIL\n",5,0);
        }

        else if(strcmp(cmd,"LIST")==0){
            send_user_list(sock);
        }

        else if(strcmp(cmd,"LOGOUT")==0){

            sscanf(buf,"%s %s",cmd,usern);

            for(int i=0;i<user_count;i++)
                if(strcmp(users[i].username,usern)==0)
                    users[i].active=0;
        }
    }

    close(sock);
    exit(0);
}

int main(){

    int server,client;
    struct sockaddr_in addr,cli;
    socklen_t len=sizeof(cli);

    server=socket(AF_INET,SOCK_STREAM,0);

    addr.sin_family=AF_INET;
    addr.sin_port=htons(PORT);
    addr.sin_addr.s_addr=INADDR_ANY;

    bind(server,(struct sockaddr*)&addr,sizeof(addr));
    listen(server,10);

    while(1){

        client=accept(server,(struct sockaddr*)&cli,&len);

        if(fork()==0){
            close(server);
            handle_client(client,cli);
        }

        close(client);
        while(waitpid(-1,NULL,WNOHANG)>0);
    }
}