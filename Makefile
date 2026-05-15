CC=gcc
CFLAGS=-Wall -pthread

all: discovery_fork discovery_thread discovery_select chat_fork chat_thread chat_select chat_client

discovery_fork: discovery_fork.c
	$(CC) $(CFLAGS) discovery_fork.c -o discovery_fork

discovery_thread: discovery_thread.c
	$(CC) $(CFLAGS) discovery_thread.c -o discovery_thread

discovery_select: discovery_select.c
	$(CC) $(CFLAGS) discovery_select.c -o discovery_select

chat_fork: chat_fork.c
	$(CC) $(CFLAGS) chat_fork.c -o chat_fork

chat_thread: chat_thread.c
	$(CC) $(CFLAGS) chat_thread.c -o chat_thread

chat_select: chat_select.c
	$(CC) $(CFLAGS) chat_select.c -o chat_select

chat_client: chat_client.c
	$(CC) $(CFLAGS) chat_client.c -o chat_client

clean:
	rm -f discovery_fork discovery_thread discovery_select chat_fork chat_thread chat_select chat_client