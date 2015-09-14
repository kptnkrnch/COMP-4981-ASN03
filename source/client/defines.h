#ifndef DEFINES_H
#define DEFINES_H

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

#endif
