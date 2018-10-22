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
int authUser(const char* message, char* user, char* pass)
{
	// Get username and password from message
	if (sscanf(message, "%s,%s", user, pass) != 2) {
		printf("%s", "Invalid user/pass format!\n");
		fflush(stdout);
		return -1;
	}
	
	// Open authentication file
	FILE* authFile = fopen("Authentication.txt", "r");
	if (authFile == NULL) {
		perror("Unable to open authentication file");
		return -1;
	}
	
	// Compare user and pass to file
	bool success = false;
	char userCmp[MAX_NAME_LENGTH], passCmp[MAX_NAME_LENGTH];
	while (!success && fscanf(authFile, "%s %s", userCmp, passCmp) != EOF) {
		// Correct username and password
		if (strcmp(userCmp, user) == 0 && strcmp(passCmp, pass) == 0) {
			success = true;
		}
	}
	
	// Failed auth
	if (!success) {
		printf("User %s failed to authenticate!\n", user);
		fflush(stdout);
		return -1;
	}
	
	// Successful auth
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


/// parseGameOption
/// Parses received string as a game option: r, f, or quit
GameOption parseGameOption(const char* buffer, int* x, int* y)
{
	// Check string matches
	if (strncmp(buffer, "quit", 4) == 0)
		return QUIT;
	else if (strncmp(buffer, "r", 1) == 0) {
		if( sscanf(buffer, "r,%d,%d", x, y) != 2) {
			printf("%s", "Invalid reveal format! Expects 'r,<x>,<y>'.\n");
			fflush(stdout);
		}
		return REVEAL;
	}
	else if (strncmp(buffer, "f", 1) == 0) {
		if (sscanf(buffer, "f,%d,%d", x, y) != 2) {
			printf("%s", "Invalid flag format! Expects 'f,<x>,<y>'.\n");
			fflush(stdout);
		}
		return FLAG;
	}

	// Invalid option
	printf("%s", "Invalid option detected. Send 'r,<x>,<y>', 'f,<x>,<y>', or 'quit'. Defaulting to 'quit'.\n");
	fflush(stdout);
	return -1;
}


/* Public functions */
/// handleConnection
/// Threaded function to handle the connection of a new user
void handleConnection(int cID)
{
	// Create buffers for incoming and outgoing messages
	char user[MAX_NAME_LENGTH], pass[MAX_NAME_LENGTH];
	char rxBuffer[MAX_RX_SIZE];
	char txBuffer[MAX_TX_SIZE];
	memset(rxBuffer, 0, sizeof(rxBuffer)/sizeof(char));
	memset(txBuffer, 0, sizeof(txBuffer)/sizeof(char));
	memset(user, 0, sizeof(user)/sizeof(char));
	memset(pass, 0, sizeof(pass)/sizeof(char));
	
	// Ensure client is still connected
	if (send(cID, "connect", 8, 0) == -1) {
		perror("Failed to confirm connection");
		closeSocket(cID);
		return;
	}
	
	// Receive username,password
	if (recv(cID, rxBuffer, MAX_RX_SIZE, 0) <= 0) {
		perror("Failed to receive data");
		closeSocket(cID);
		return;
	}
	
	//Authenticate user
	if(authUser(rxBuffer, user, pass) == 0) {
		if (send(cID, "accept", 7, 0)) {
			perror("Failed to send data (accept user auth)");
			closeSocket(cID);
			return;
		}
	} else {
		closeSocket(cID);
		return;
	}
	
	// Keep receiving until user disconnects
	bool exit = false;
	while (!exit) {
		// Receive a menu option, "play", "lb" or "exit"
		if (recv(cID, rxBuffer, MAX_RX_SIZE, 0) <= 0) {
			perror("Failed to receive data");
			break;
		}
	
		// Parse menu option
		switch parseMenuOption(rxBuffer):
			case PLAY:
				// Accept game start
				if (send(cID, "accept", 7, 0)) {
					perror("Failed to send data (accept game start)");
					exit = true;
					break;
				}
			
				// Enter game loop
				GameState game; initGame(&game);
				while (!game->isOver) {
					// Receive game option
					if (recv(cID, rxBuffer, MAX_RX_SIZE, 0) <= 0) {
						perror("Failed to receive data");
						exit = true;
						break;
					}
					
					// Parse game option
					int x, int y;
					switch parseGameOption(rxBuffer, &x, &y):
						case REVEAL:
							int txLen = requestReveal(&game, x, y, txBuffer);
							// Mine hit!
							if (txLen == 0) {
								// Store new record and set transmit message
								long int gameTime = (long int)difftime(game->endTime, game->startTime);
								newRecord(user, false, gameTime);
								sprintf(txBuffer, "over,0,%ld", gameTime);
								txLen = strlen(txBuffer);
								
								// Send message
								if (send(cID, txBuffer, txLen, 0) == -1) {
									perror("Failed to send data (reveal game tile)");
									game->isOver = true;
									exit = true;
								}
								
								// Wait for OK
								if (recv(cID, rxBuffer, MAX_RX_SIZE, 0) <= 0) {
									perror("Failed to receive data");
									exit = true;
									break;
								}
								
								// TODO:
								// Send ALL tiles
							}
							
							// Send reply
							else if (send(cID, txBuffer, txLen, 0) == -1) {
								perror("Failed to send data (reveal game tile)");
								game->isOver = true;
								exit = true;
							}
							break;
						
						case FLAG:
							int txLen = requestFlag(&game, x, y, txBuffer);
							// Game won!
							if (txLen == 0) {
								// Store new record and set transmit message
								long int gameTime = (long int)difftime(game->endTime, game->startTime);
								newRecord(user, true, gameTime);
								sprintf(txBuffer, "over,1,%ld", gameTime);
								txLen = strlen(txBuffer);
							}
							
							// Send reply
							if (send(cID, txBuffer, txLen, 0) == -1) {
								perror("Failed to send data (flag game tile)");
								game->isOver = true;
								exit = true;
							}
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
				if (txLen == 0) {
					// No leaderboard data
					if (send(cID, "error", 6, 0) == -1) {
						perror("Failed to send data (leaderboard)");
						exit = true;
					}
				}
				else if (send(cID, txBuffer, txLen, 0) == -1) {
						perror("Failed to send data (leaderboard)");
						exit = true;
				}
				break;
				
			case EXIT:
			default:
				exit = true;
				break;
	}
	
	// User has exitied, or something went wrong
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