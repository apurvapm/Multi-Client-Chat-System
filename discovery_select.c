// discovery_select.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/select.h>

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
int user_count=0;

void send_user_list(int sock){

    char buf[BUF];

    for(int i=0;i<user_count;i++)
        if(users[i].active){
            sprintf(buf,"USER %s %s %d\n",
                    users[i].username,
                    users[i].ip,
                    users[i].port);
            send(sock,buf,strlen(buf),0);
        }

    send(sock,"END\n",4,0);
}

int main(){

    int server,client;
    struct sockaddr_in addr,cli;

    fd_set master,readfds;
    int fdmax;

    server=socket(AF_INET,SOCK_STREAM,0);

    addr.sin_family=AF_INET;
    addr.sin_port=htons(PORT);
    addr.sin_addr.s_addr=INADDR_ANY;

    bind(server,(struct sockaddr*)&addr,sizeof(addr));
    listen(server,10);

    FD_ZERO(&master);
    FD_SET(server,&master);
    fdmax=server;

    while(1){

        readfds=master;
        select(fdmax+1,&readfds,NULL,NULL,NULL);

        for(int i=0;i<=fdmax;i++){

            if(!FD_ISSET(i,&readfds)) continue;

            if(i==server){

                socklen_t len=sizeof(cli);
                client=accept(server,(struct sockaddr*)&cli,&len);

                FD_SET(client,&master);
                if(client>fdmax) fdmax=client;
            }

            else{

                char buf[BUF];
                int n=recv(i,buf,BUF-1,0);

                if(n<=0){
                    close(i);
                    FD_CLR(i,&master);
                    continue;
                }

                buf[n]=0;

                char cmd[50],u[50],p[50];
                int port;

                sscanf(buf,"%s",cmd);

                if(strcmp(cmd,"REGISTER")==0){

                    sscanf(buf,"%s %s %s %d",cmd,u,p,&port);

                    strcpy(users[user_count].username,u);
                    strcpy(users[user_count].password,p);
                    users[user_count].port=port;
                    users[user_count].active=1;

                    user_count++;

                    send(i,"OK\n",3,0);
                }

                else if(strcmp(cmd,"LOGIN")==0){

                    sscanf(buf,"%s %s %s",cmd,u,p);

                    int found=0;

                    for(int j=0;j<user_count;j++)
                        if(strcmp(users[j].username,u)==0 &&
                           strcmp(users[j].password,p)==0){
                            users[j].active=1;
                            found=1;
                        }

                    if(found) send(i,"OK\n",3,0);
                    else send(i,"FAIL\n",5,0);
                }

                else if(strcmp(cmd,"LIST")==0)
                    send_user_list(i);

                else if(strcmp(cmd,"LOGOUT")==0){

                    sscanf(buf,"%s %s",cmd,u);

                    for(int j=0;j<user_count;j++)
                        if(strcmp(users[j].username,u)==0)
                            users[j].active=0;
                }
            }
        }
    }
}