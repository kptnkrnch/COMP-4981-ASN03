/*---------------------------------------------------------------------------------------
--	SOURCE FILE:		svr.cpp - Linux console chat program.
--
--	PROGRAM:		svr
--
--	FUNCTIONS:		
--
--	DATE:			March 22, 2014
--
--	REVISIONS:		
--
--
--	DESIGNERS:		Josh Campbell and Ian Davidson
--
--	PROGRAMMERS:		Ian Davidson
--
--	NOTES:
--	This program will handle accepting new clients. It will handle forwarding data to the
--  connected clients. It will handle receiving data from the clients to be forwarded.
--  The program also handles removing clients upon receiving a disconnect packet.
---------------------------------------------------------------------------------------*/

#include <iostream>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <strings.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdint.h>
#include <cstring>

#define SERVER_TCP_PORT     7000	//Default port
#define BUFLEN	            2000	//Buffer length
#define SYN                 0x16    //sync
#define EOT                 0x04    //end of transmission
#define SERVER_NAME_MSG     0x01    //server name chat room message
#define NEW_CLIENT_MSG      0x02    //new client message
#define CHAT_MSG            0x03    //a chat message
#define CLIENT_REMOVED_MSG  0x04    //client removed message
#define ROOM_NAME_LEN       50      //max length of a chat room name
#define CLIENT_NAME_LEN     50      //max length of a client name
#define DATALEN             BUFLEN - CLIENT_NAME_LEN - 11   //maximum amount of data
#define MAX_CLIENTS         30      //max number clients able to be connected

using namespace std;
void Create_ServerName_Packet(uint8_t ** buffer, uint32_t bufferSize, char * servername, uint32_t namesize);

/*----------------------------------------------------------------------
--FUNCTION: Create_ServerName_Packet
--
--DATE: March 22, 2014
--
--REVISIONS: none
--
--DESIGNER: Joshua Campbell and Ian Davidson
--
--PROGRAMMER: Ian Davidson
--
--PARAMS: uint8_t ** buffer - the packet that is going to be formatted
							  with the disconnect data.
--		  uint32_t buffersize - max amount of data in the buffer array
--		  char * servername - pointer to the array containing the server's
							  name.
--		  uint32_t namesize - the number of characters in the name/alias.
--
--RETURNS: void
--
--NOTES:
--Function for creating a packet that alerts the server that a new client
--(this client) has joined the chat.
----------------------------------------------------------------------*/
void Create_ServerName_Packet(uint8_t ** buffer, uint32_t bufferSize, char * servername, uint32_t namesize) {
    uint32_t MAXNAMESIZE = 50;
    uint32_t pos = 0;
    uint32_t shift = 0;
    (*buffer)[pos] = 0x16;
    pos++;
    (*buffer)[pos] = 0x01;
    pos++;
    namesize++;
    for (uint32_t i = 0, shift = 24; i < 4 && shift >= 0; i++) {
        (*buffer)[pos] = (uint8_t)(namesize >> shift);
        pos++;
        shift -= 8;
    }

    for (uint32_t i = 0; i < namesize; i++) {
        (*buffer)[pos] = servername[i];
        pos++;
    }
    (*buffer)[pos] = '\0';
    pos++;

    for (uint32_t i = 0; i < (MAXNAMESIZE - namesize); i++) {
        (*buffer)[pos] = 0;
        pos++;
    }
    (*buffer)[pos] = 0x04;
    pos++;

    for (uint32_t i = pos; i < bufferSize; i++) {
        (*buffer)[i] = 0;
    }
}


/*----------------------------------------------------------------------
--FUNCTION: main
--
--DATE: March 22, 2014
--
--REVISIONS: none
--
--DESIGNER: Joshua Campbell and Ian Davidson
--
--PROGRAMMER: Ian Davidson
--
--PARAMS: none
--
--RETURNS: int
--
--NOTES:
--Main program, handles creating packets.
--handles accepting clients.
--handles forwarding data to clients.
--handles disconnecting clients.
----------------------------------------------------------------------*/
int main()
{
    int i, maxi, nready, bytes_to_read, arg;
	int listen_sd, new_sd, sockfd, socktmp, client_len, maxfd, client[FD_SETSIZE];
	struct sockaddr_in server, client_addr[MAX_CLIENTS];
	uint8_t *bp, buf[BUFLEN], roomName[ROOM_NAME_LEN], data[DATALEN], *sendBuffer;
	char clientName[CLIENT_NAME_LEN];
	ssize_t n;
	uint32_t namesize = 0;
	uint32_t pos = 0;
    int clientNum = 0;
   	fd_set rset, allset;
   	
    int port = SERVER_TCP_PORT;
    printf("Please enter a port #: ");
    cin >> port;
    
    
    if ((listen_sd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("listen socket failed to create.\n");
        exit(1);
    }

    arg = 1;
    if (setsockopt (listen_sd, SOL_SOCKET, SO_REUSEADDR, &arg, sizeof(arg)) == -1) {
        perror("setsoctopt failed.\n");
        exit(1);
    }

    bzero((char *)&server, sizeof(struct sockaddr_in));
	server.sin_family = AF_INET;
	server.sin_port = htons(port);
	server.sin_addr.s_addr = htonl(INADDR_ANY); // Accept connections from any client

    if (bind(listen_sd, (struct sockaddr *)&server, sizeof(server)) == -1) {
        perror("bind failed.\n");
        exit(1);
    }
    listen(listen_sd, 5);

    maxfd	= listen_sd;	// initialize
   	maxi	= -1;		// index into client[] array

   	for (i = 0; i < FD_SETSIZE; i++) {
           	client[i] = -1;             // -1 indicates available entry
    }

    FD_ZERO(&allset);
   	FD_SET(listen_sd, &allset);
    sendBuffer = (uint8_t*)malloc(sizeof(uint8_t)*BUFLEN);
    
   	while (true) {
        rset = allset;               // structure assignment
        nready = select(maxfd + 1, &rset, NULL, NULL, NULL);

        if (FD_ISSET(listen_sd, &rset)) { // new client connection
            client_len = sizeof(client_addr);
            if ((new_sd = accept(listen_sd, (struct sockaddr *) &client_addr[clientNum], (socklen_t *)&client_len)) == -1) {
                perror("accept error.\n");
                exit(1);
            }
            
            printf("Remote Address:  %s\n", inet_ntoa(client_addr[clientNum].sin_addr));
            
            Create_ServerName_Packet(&sendBuffer, BUFLEN, "Room 1", (uint32_t)strlen("Room 1"));
            
            write(new_sd, sendBuffer, BUFLEN); //send the room name packet to the new client
            
            for (i = 0; i < FD_SETSIZE; i++) {
                if (client[i] < 0) {
                    client[i] = new_sd;	// save descriptor
                    break;
                }
            }
            if (i == FD_SETSIZE) {
                printf ("Too many clients\n");
                exit(1);
            }

            FD_SET (new_sd, &allset);     // add new descriptor to set
            if (new_sd > maxfd) {
                maxfd = new_sd;	// for select
            }

            if (i > maxi) {
                maxi = i;	// new max index in client[] array
            }

            if (--nready <= 0)
                continue;	// no more readable descriptors
        }
       // printf( "here 4, totally not creating a new socket\n");
        for (i = 0; i <= maxi; i++)	{ // check all clients for data
            if ((sockfd = client[i]) < 0) {
                continue;
            }
            if (FD_ISSET(sockfd, &rset)) {
                bp = buf;
                bytes_to_read = BUFLEN;
                while ((n = read(sockfd, bp, bytes_to_read)) > 0) {
                    bp += n;
                    bytes_to_read -= n;
                }
                pos = 0;
                if(buf[0] == SYN){
                    pos++;
                    switch(buf[1]){
                        case SERVER_NAME_MSG: 
                        break;
                        case NEW_CLIENT_MSG: //we recieved a new client message
                        pos++;
                        namesize = ((uint32_t)buf[2] << 24 | 
                                    (uint32_t)buf[3] << 16 |
                                    (uint32_t)buf[4] << 8 |
                                    (uint32_t)buf[5] << 0);
                        pos += 4;
                        for(int k = 0; k < (int)namesize; k++){ //get the clientname
                            clientName[k] = (char)buf[pos];
                            pos++;
                        }
                                    
                        printf("NAMESIZE: %d\n", (int)namesize);
                        printf("%s has connected\n", clientName);
                        for (int j = 0; j <= maxi; j++)	{ //go through all clients
                            if ((socktmp = client[j]) < 0) {
                                continue;
                            } 
                            if(socktmp != sockfd){ //make sure we dont send to original client
                                write(socktmp, buf, BUFLEN); //send all other clients the msg
                            }
                        }
                        break;
                        case CHAT_MSG: //we recieved a chat message
                        printf( "%s recieved from socket\n", buf);
                        pos++;
                        namesize = ((uint32_t)buf[2] << 24 | 
                                    (uint32_t)buf[3] << 16 |
                                    (uint32_t)buf[4] << 8 |
                                    (uint32_t)buf[5] << 0);
                        pos += 4;
                        for(int k = 0; k < (int)namesize; k++){ //get the clientname
                            clientName[k] = (char)buf[pos];
                            pos++;
                        }
                        for (int j = 0; j <= maxi; j++)	{ //go through all clients
                            if ((socktmp = client[j]) < 0) {
                                continue;
                            } 
                            if(socktmp != sockfd){
                                printf("forwarded %s's packet\n", clientName);
                                write(socktmp, buf, BUFLEN); //send all other clients the msg
                            }
                        }
                        break;
                        case CLIENT_REMOVED_MSG: //we recieved a client removed message
                        printf( "data recieved from socket\n");
                        pos++;
                        namesize = ((uint32_t)buf[2] << 24 | 
                                    (uint32_t)buf[3] << 16 |
                                    (uint32_t)buf[4] << 8 |
                                    (uint32_t)buf[5] << 0);
                        pos += 4;
                        for(int k = 0; k < (int)namesize; k++){ //get the clientname
                            clientName[k] = (char)buf[pos];
                            pos++;
                        }
                        printf("%s has disconnected\n", clientName);
                        for (int j = 0; j <= maxi; j++)	{ //go through all clients
                            if ((socktmp = client[j]) < 0) {
                                continue;
                            } 
                            if(socktmp != sockfd){ //make sure we dont send to original client
                                write(socktmp, buf, BUFLEN); //send all other clients the msg
                            }
                        }
                       
                        close(sockfd);
                        FD_CLR(sockfd, &allset);
                        client[i] = -1;
                        clientNum--;
                        break;
                    }
                } else {
                    fprintf(stderr, "SYN bit not set\n");
                    write(sockfd, buf, BUFLEN);   // echo to client
                }
                

                if (--nready <= 0) {
                    break;        // no more readable descriptors
                }
            }
        }
        clientNum++;
    }
    return 0;
}
