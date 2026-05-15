#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define DISCOVERY_PORT 9000
#define BUFFER 1024

int connect_server(char *ip, int port)
{
    int sock;
    struct sockaddr_in addr;

    sock = socket(AF_INET, SOCK_STREAM, 0);

    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    inet_pton(AF_INET, ip, &addr.sin_addr);

    if(connect(sock,(struct sockaddr*)&addr,sizeof(addr))<0)
    {
        perror("connect");
        exit(1);
    }

    return sock;
}

int main()
{
    char buffer[BUFFER];
    char username[32], password[32];
    char chat_ip[32];
    int chat_port;

    printf("Username: ");
    scanf("%s",username);

    printf("Password: ");
    scanf("%s",password);

    getchar();

    /* connect discovery */
    int dsock = connect_server("127.0.0.1",DISCOVERY_PORT);

    sprintf(buffer,"REGISTER %s %s\n",username,password);
    send(dsock,buffer,strlen(buffer),0);

    recv(dsock,buffer,BUFFER,0);
    printf("%s",buffer);

    send(dsock,"GET_SERVER\n",11,0);

    recv(dsock,buffer,BUFFER,0);
    sscanf(buffer,"CHAT_SERVER %s %d",chat_ip,&chat_port);

    close(dsock);

    /* connect chat server */

    int csock = connect_server(chat_ip,chat_port);

    sprintf(buffer,"LOGIN %s %s\n",username,password);
    send(csock,buffer,strlen(buffer),0);

    recv(csock,buffer,BUFFER,0);
    printf("%s",buffer);

    printf("\nCommands:\n");
    printf("BROADCAST msg\n");
    printf("PRIVATE user msg\n");
    printf("LIST\n");
    printf("LOGOUT\n\n");

    fd_set readfds;

    while(1)
    {
        FD_ZERO(&readfds);
        FD_SET(0,&readfds);
        FD_SET(csock,&readfds);

        select(csock+1,&readfds,NULL,NULL,NULL);

        if(FD_ISSET(0,&readfds))
        {
            fgets(buffer,BUFFER,stdin);
            send(csock,buffer,strlen(buffer),0);
        }

        if(FD_ISSET(csock,&readfds))
        {
            int n = recv(csock,buffer,BUFFER-1,0);
            if(n<=0) break;

            buffer[n]=0;
            printf("%s",buffer);
        }
    }

    close(csock);
}