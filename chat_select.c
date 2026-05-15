// chat_select.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/select.h>

#define PORT 8000
#define BUF 1024
#define MAX_CLIENTS 100

typedef struct{
    int sock;
    char username[50];
} client;

client clients[MAX_CLIENTS];
int count=0;

void broadcast(char *msg,int sender){

    for(int i=0;i<count;i++)
        if(clients[i].sock!=sender)
            send(clients[i].sock,msg,strlen(msg),0);
}

void private_msg(char *user,char *msg){

    for(int i=0;i<count;i++)
        if(strcmp(clients[i].username,user)==0)
            send(clients[i].sock,msg,strlen(msg),0);
}

int main(){

    int server,newfd;
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
                newfd=accept(server,(struct sockaddr*)&cli,&len);

                FD_SET(newfd,&master);
                if(newfd>fdmax) fdmax=newfd;
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

                char cmd[50],user[50],msg[900];
                sscanf(buf,"%s",cmd);

                if(strcmp(cmd,"LOGIN")==0){

                    sscanf(buf,"%s %s",cmd,user);

                    clients[count].sock=i;
                    strcpy(clients[count].username,user);
                    count++;
                }

                else if(strcmp(cmd,"BROADCAST")==0){

                    strcpy(msg,buf+10);
                    broadcast(msg,i);
                }

                else if(strcmp(cmd,"MSG")==0){

                    sscanf(buf,"%s %s %[^\n]",cmd,user,msg);
                    private_msg(user,msg);
                }
            }
        }
    }
}