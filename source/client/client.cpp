/*---------------------------------------------------------------------------------------
--	SOURCE FILE:		client.cpp - Linux console chat program.
--
--	PROGRAM:		client
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
--	PROGRAMMERS:		Josh Campbell
--
--	NOTES:
--	This program will establish a connection with the chat server, the user will set
--  their username/alias which will be sent to the server to be sent to the other clients
--  After receiving the server name message, the client can then type in text to send to
--  the other clients. It will also receive chat messages, disconnected client messages,
--  and client joining messages during this time. typing in --d and then pressing enter
--  will send a disconnect message to the server (ctrl + c) works as well.
---------------------------------------------------------------------------------------*/
#include <stdio.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <stdlib.h>
#include <strings.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <signal.h>
#include <stdint.h>
#include <cstring>
#include <cstdlib>
#include <pthread.h>
#include <string>
#include <iostream>
#include <fstream>
#include "defines.h"
#include "sending.h"
#include "receiving.h"

#define SERVER_TCP_PORT		7000	// Default port
#define BUFLEN			    2000  	// Buffer length
#define USERNAME_BUF        200
#define MAXNAMESIZE         50
#define MAXMESSAGESIZE      BUFLEN - MAXNAMESIZE - 11
#define SYN                 0x16
#define EOT                 0x04
#define SERVERNAME          0x01
#define CLIENTNAME          0x02
#define CHATMESSAGE         0x03
#define CLIENTREMOVED       0x04

using namespace std;

void * ReadMessageThread(void * tdata);

void catch_int(int);
void catch_term(int signo);

typedef struct {
    int sd; //socket descriptor
    char * username;
    int namesize;
    ofstream * file;
    bool saveToFile;
} ThreadData;

int gSD = 0;
char * gUserName = 0;
ofstream * gFile = 0;

/*----------------------------------------------------------------------
--FUNCTION: main
--
--DATE: March 22, 2014
--
--REVISIONS: none
--
--DESIGNER: Joshua Campbell and Ian Davidson
--
--PROGRAMMER: Joshua Campbell
--
--PARAMS: int argc - the argument count
--		  char **argv - the arguments, used for saving to file.
--
--RETURNS: int
--
--NOTES:
--Main program, handles input from the user.
--Handles creating the file to save.
--Handles creating the socket to the server.
--Creates the reading thread.
----------------------------------------------------------------------*/
int main (int argc, char **argv) {
    signal(SIGINT, catch_int);
    signal(SIGTERM, catch_term);
	int n, bytes_to_read;
	int sd, port;
	struct hostent	*hp;
	struct sockaddr_in server;
	char sbuf[BUFLEN], **pptr, msgbuf[BUFLEN], clientname[USERNAME_BUF];
	char str[16];
	bool disconnect = false;
	char usernm[MAXNAMESIZE];
	char message[MAXMESSAGESIZE];
	ThreadData tdata;
	char host[20];
	char temp[20];
	ofstream file;
	bool saveToFile = false;

	tdata.username = (char *)malloc(sizeof(char) * MAXNAMESIZE);

    pthread_t * reader = (pthread_t *)malloc(sizeof(pthread_t));

	fd_set rset, allset;

	uint8_t rbuf[BUFLEN];
	uint8_t *bp;

	switch(argc)
	{
		case 2: //if we receive an argument save text to a file
			saveToFile = true;
			file.open(argv[1], ios::out | ios::trunc ); //open/create a new file to append
			printf("Saving chat to file: %s\n", argv[1]);
			gFile = &file;
		break;
        case 1: //if we recieve no argument do not save text to a file
            saveToFile = false;
            printf("Not saving to a file.\n");
        break;
		default: //error if we receive more than one argument
			fprintf(stderr, "Usage: %s [fileName.txt]\n       or %s \n", argv[0], argv[0]);
            exit(1);
	}

    printf("Enter Host IP: ");
    fgets(host, 20, stdin);

    for (int index = 0; index < strlen(host); index++) {
        if (host[index] == '\n') {
            host[index] = '\0';
            break;
        }
	}

    printf("Enter Host Port: ");
    fgets(temp, 20, stdin);
    sscanf(temp, "%d", &port);

    printf("Enter Username: ");
    fgets(usernm, MAXNAMESIZE, stdin);
    gUserName = usernm;

	for (int index = 0; index < strlen(usernm); index++) {
        if (usernm[index] == '\n') {
            usernm[index] = '\0';
            break;
        }
	}

	// Create the socket
	if ((sd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		perror("Cannot create socket");
		exit(1);
	}
	gSD = sd;

	bzero((char *)&server, sizeof(struct sockaddr_in));
	server.sin_family = AF_INET;
	server.sin_port = htons(port);

	if ((hp = gethostbyname(host)) == NULL) {
		fprintf(stderr, "Unknown server address\n");
		exit(1);
	}
	bcopy(hp->h_addr, (char *)&server.sin_addr, hp->h_length);


	// Connecting to the server
	if (connect (sd, (struct sockaddr *)&server, sizeof(server)) == -1)
	{
		fprintf(stderr, "Can't connect to server\n");
		perror("connect");
		exit(1);
	}
	printf("Connected:    Server Name: %s\n", hp->h_name);
	pptr = hp->h_addr_list;
	printf("\t\tIP Address: %s\n", inet_ntop(hp->h_addrtype, *pptr, str, sizeof(str)));
	
    uint8_t * datapacket;

    datapacket = (uint8_t *)malloc(sizeof(uint8_t) * BUFLEN);

    Create_ClientName_Packet(&datapacket, BUFLEN, usernm, (uint32_t)strlen(usernm));

	// Transmit data through the socket
	send (sd, datapacket, BUFLEN, 0);

	tdata.sd = sd;
	strcpy(tdata.username, usernm);
	tdata.namesize = strlen(usernm);
	if (saveToFile) {
        tdata.file = &file;
        tdata.saveToFile = saveToFile;
	} else {
        tdata.file = 0;
        tdata.saveToFile = saveToFile;
	}
	/*printf("Receive:\n");
	bp = rbuf;
	bytes_to_read = BUFLEN;*/
    pthread_create(reader, NULL, ReadMessageThread, &tdata);
	// client makes repeated calls to recv until no more data is expected to arrive.
    while(!disconnect) {
        //printf("%s: ", usernm);
        fgets(message, MAXMESSAGESIZE, stdin);
        for (int i = 0; i < strlen(message) - 2; i++) {
            if (message[i] == '-') {
                if (message[i + 1] == '-') {
                    if (message[i + 2] == 'D' || message[i + 2] == 'd') {
                        disconnect = true;
                        break;
                    }
                }
            }
        }
        Create_ChatMessage_Packet(&datapacket, BUFLEN, usernm, (uint32_t)strlen(usernm), message, (uint32_t)strlen(message));
        send(sd, datapacket, BUFLEN, 0);
        printf("\n");
        file << usernm << ": " << message << "\n";
    }

	Create_Disconnect_Packet(&datapacket, BUFLEN, usernm, (uint32_t)strlen(usernm));
	// Transmit data through the socket
	send (sd, datapacket, BUFLEN, 0);

	//printf ("%s\n", rbuf);
	fflush(stdout);
	close (sd);
	free(datapacket);
	free(reader);
	free(tdata.username);
	if (file.is_open()) {
        file.close();
	}
	return (0);
}

/*----------------------------------------------------------------------
--FUNCTION: catch_int
--
--DATE: March 22, 2014
--
--REVISIONS: none
--
--DESIGNER: Joshua Campbell
--
--PROGRAMMER: Joshua Campbell
--
--PARAMS: int signo - the id of the interrupt caught
--
--RETURNS: void
--
--NOTES:
--Handles sending disconnect packet when exiting the program.
----------------------------------------------------------------------*/
void catch_int(int signo) {
    uint8_t * datapacket;

    datapacket = (uint8_t *)malloc(sizeof(uint8_t) * BUFLEN);
    Create_Disconnect_Packet(&datapacket, BUFLEN, gUserName, (uint32_t)strlen(gUserName));
	// Transmit data through the socket
	send (gSD, datapacket, BUFLEN, 0);
	free(datapacket);
	gFile->close();
    exit(0);
}

/*----------------------------------------------------------------------
--FUNCTION: catch_term
--
--DATE: March 22, 2014
--
--REVISIONS: none
--
--DESIGNER: Joshua Campbell
--
--PROGRAMMER: Joshua Campbell
--
--PARAMS: int signo - the id of the interrupt caught
--
--RETURNS: void
--
--NOTES:
--Handles sending disconnect packet when exiting the program.
----------------------------------------------------------------------*/
void catch_term(int signo) {
    uint8_t * datapacket;

    datapacket = (uint8_t *)malloc(sizeof(uint8_t) * BUFLEN);
    Create_Disconnect_Packet(&datapacket, BUFLEN, gUserName, (uint32_t)strlen(gUserName));
	// Transmit data through the socket
	send (gSD, datapacket, BUFLEN, 0);
	free(datapacket);
	gFile->close();
    exit(0);
}

/*----------------------------------------------------------------------
--FUNCTION: ReadMessageThread
--
--DATE: March 22, 2014
--
--REVISIONS: none
--
--DESIGNER: Josh Campbell
--
--PROGRAMMER: Joshua Campbell
--
--PARAMS: int msgID - void * tdata - The thread data structure,
							contains the socket, file to write to,
							username and size.
--
--RETURNS: void *
--
--NOTES:
--This is the main reading thread, it reads in data from the socket and
--then passes it to the proper extraction function for printing/saving.
----------------------------------------------------------------------*/
void * ReadMessageThread(void * tdata) {
    ThreadData * data = (ThreadData *) tdata;
    char  *host, sbuf[BUFLEN], **pptr, msgbuf[BUFLEN], clientname[USERNAME_BUF];
	char str[16];
	bool disconnect = false;

	uint8_t rbuf[BUFLEN];
	uint8_t *bp;

    int n = 0, bytes_to_read = 0;

	while (!disconnect) {
        bytes_to_read = BUFLEN;
        bp = rbuf;
        while ((n = recv (data->sd, bp, bytes_to_read, 0)) < BUFLEN)
        {
            bp += n;
            bytes_to_read -= n;
        }

        if (rbuf[0] == SYN) {
            switch(rbuf[1]) {
                case SERVERNAME:
                    ReadServerName(rbuf, BUFLEN, data->file, data->saveToFile);
                break;
                case CLIENTNAME:
                    ReadClientName(rbuf, BUFLEN, data->file, data->saveToFile);
                break;
                case CHATMESSAGE:
                    ReadChatMessage(rbuf, BUFLEN, data->file, data->saveToFile);
                break;
                case CLIENTREMOVED:
                    ReadClientDisconnect(rbuf, BUFLEN, data->file, data->saveToFile);
                break;
            }
        } else {
            perror("Did not receive SYN.\n");
        }

	}
}
