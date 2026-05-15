# Multi-Client Chat Application

This project implements a TCP-based multi-client chat system with a discovery service.

The system demonstrates three server concurrency models:

• Fork based server  
• Thread based server  
• Select based server

Programs included:

discovery_fork.c  
discovery_thread.c  
discovery_select.c  
chat_fork.c  
chat_thread.c  
chat_select.c  
chat_client.c


----------------------------------------

System Architecture

Client → Discovery Server → Chat Server

The discovery server acts like DNS and maps:

username → IP address + port

The chat server manages active connections and message exchange.

----------------------------------------

Protocol Design

TCP is used for reliable communication.

To handle variable length messages, we use **length-prefixed framing**.

Message format:

<length>\n<payload>

Example:

20
LOGIN user1 pass

The receiver:

1. Reads the length header
2. Reads exactly that many bytes
3. Parses the payload

----------------------------------------

Discovery Server Commands

REGISTER username password port

Registers a user.

Response:
OK
or
FAIL UsernameExists


LOGIN username password

Authenticates user.

Response:
OK
or
FAIL InvalidCredentials


LIST

Returns active users.

Response format:

USER username ip port
USER username ip port
END


LOGOUT username

Marks user inactive.


----------------------------------------

Chat Server Commands

LOGIN username

User joins the chat server.


BROADCAST message

Message is delivered to all connected users.


MSG username message

Send private message to a specific user.


LIST

Returns currently connected users.


LOGOUT

User disconnects.


----------------------------------------

Compilation

Run:

make


----------------------------------------

Running the System

Start discovery server:

./discovery_thread

Start chat server:

./chat_thread


Run clients:

./chat_client


----------------------------------------

Concurrency Models

Fork server

Each client connection creates a new process.


Thread server

Each client handled by a separate thread.


Select server

Single process multiplexes clients using select().


----------------------------------------

Features

• Multi-client messaging  
• Broadcast and private messages  
• Active user discovery service  
• Three concurrency models  
• TCP reliable transport  
• Proper message framing

----------------------------------------
