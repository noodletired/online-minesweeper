/* * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * server/comms.c
 * Minesweeper server communication functions
 *
 * Author:  Keagan Godfrey
 * Version: 1.0
 * Date:    20/10/2018
 * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/* Include directives */
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "comms.h"
#include "leaderboard.h"
#include "minesweeper.h"


/* Private functions */
/// authUser
/// Authenticate user connection
int authUser(const char* buffer)
{
	// Create buffers for incoming messages
	char user[MAX_NAME_LENGTH], pass[MAX_NAME_LENGTH];
	memset(user, 0, sizeof(user)/sizeof(char));
	memset(pass, 0, sizeof(pass)/sizeof(char));
	
	
	else {
		printf("User %s failed to authenticate!\n", user);
		fflush(stdout);
		return -1;
	}
	
	// TODO
	return 0;
}


/// parseMenuOption
/// Parses received string as a menu option: play, lb, or exit
MenuOption parseMenuOption(const char* buffer)
{
	// Check string matches
	if (strncmp(buffer, "play", 4) == 0)
		return PLAY;
	else if (strncmp(buffer, "lb", 2) == 0)
		return LB;
	else if (strncmp(buffer, "exit", 4) == 0)
		return EXIT;
	
	// Invalid option
	printf("%s", "Invalid option detected. Send 'play', 'lb', or 'exit'. Defaulting to 'exit'.\n");
	fflush(stdout);
	return -1;
}


/* Public functions */
/// handleConnection
/// Threaded function to handle the connection of a new user
void handleConnection(int cID)
{
	// Create buffers for incoming and outgoing messages
	char rxBuffer[MAX_RX_SIZE];
	char txBuffer[MAX_TX_SIZE];
	memset(rxBuffer, 0, sizeof(rxBuffer)/sizeof(char));
	memset(txBuffer, 0, sizeof(txBuffer)/sizeof(char));
	
	// Ensure client is still connected
	if (send(cID, "connect", 8, 0) == -1) {
		closeSocket(cID);
		perror("Failed to confirm connection");
		return;
	}
	
	// Receive username,password
	if (recv(cID, rxBuffer, <= 0) {
		closeSocket(cID);
		perror("Failed to receive data");
		return;
	}
	
	//Authenticate user
	if(authUser(rxBuffer) == 0) {
		send(cID, "accept", 7, 0);
	} else {
		closeSocket(cID);
		return;
	}
	
	// Keep receiving until user disconnects
	bool exit = false;
	while (!exit) {
		// Receive a menu option, "play", "lb" or "exit"
		if (recv(cID, rxBuffer, <= 0) {
			perror("Failed to receive data");
			break;
		}
	
		// Parse menu option
		switch parseMenuOption(rxBuffer):
			case PLAY:
				// Enter game loop
				GameState game; initGame(&game);
				while (!game->isOver) {
					// Receive game option
					if (recv(cID, rxBuffer, <= 0) {
						perror("Failed to receive data");
						exit = true;
						break;
					}
					
					// Parse game option
					// TODO
					switch parseGameOption(rxBuffer):
						case REVEAL:
							break;
							
						case FLAG:
							break;
							
						case QUIT:
						default:
							// End game abruptly
							game->isOver = true;
							break;
				}
				break;
				
			case LB:
				// Display leaderboard
				int txLen = requestLeaderboard(txBuffer);
				if (send(cID, txBuffer, txLen, 0) == -1) {
					perror("Failed to send leaderboard data");
					exit = true;
				}
				break;
				
			case EXIT:
			default:
				exit = true;
				break;
	}
	
	// User has exit, or something went wrong
	closeSocket(cID);
}


/// openSocket
/// Opens, binds and allows listening on a defined port
int openSocket(int port)
{
	// Create socket
	int sID = socket(AF_INET, SOCK_STREAM, 0);
	if (sID == -1) {
		perror("Failed to open socket");
		exit(1); // error
	}
	
	// Define endpoint
	struct sockaddr_in sAddr;
	memset(&sAddr, 0, sizeof(sAddr));
	sAddr.sin_family = AF_INET;
	sAddr.sin_port = htons(port);
	sAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	
	// Bind socket
	int bErr = bind(sID, (struct sockaddr*)&sAddr, sizeof(sAddr));
	if (bErr == -1) {
		perror("Failed to bind socket");
		exit(1); // error
	}
	
	// Listen on socket
	int lErr = listen(sID, BACKLOG);
	if (lErr == -1) {
		perror("Failed to listen on socket");
		close(sID);
		exit(1); // error
	}
	
	printf("Server listening on port %d...\n", port);
	return sID;
}


/// closeSocket
/// Safely shuts down and closes socket defined by sID
void closeSocket(int sID)
{
	// Shutdown socket
	int sErr = shutdown(sID, SHUT_RDWR);
	if (sErr == -1) {
		perror("Failed to shutdown socket");
		return;
	}

	// Close socket
	int cErr = close(sID);
	if (cErr == -1) {
		perror("Failed to close socket");
		return;
	}
}