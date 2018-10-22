/* * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * server/comms.h
 * Header for server-side communications
 *
 * Author:  Keagan Godfrey
 * Version: 1.0
 * Date:    20/10/2018
 * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef __server_comms__h__
#define __server_comms__h__


/* Defines */
#define MAX_RX_SIZE 10
#define MAX_TX_SIZE 1000
#define MAX_NAME_LENGTH 20
#define BACKLOG 10


/* Types */
typedef enum {EXIT, PLAY, LB} MenuOption;
typedef enum {QUIT, REVEAL, FLAG} GameOption;


/* Public function prototypes */
/// handleConnection
/// Threaded function to handle the connection of a new user
void handleConnection(int cID);


/// openSocket
/// Opens, binds and allows listening on a defined port
int openSocket(int port);


/// closeSocket
/// Safely shuts down and closes socket defined by sID
void closeSocket(int sID);


#endif