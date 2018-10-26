/* * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * client/main.c
 * Minesweeper client main entrypoint
 *
 * Author:  Keagan Godfrey / Christopher Dare
 * Version: 1.0
 * Date:    24/10/2018
 * * * * * * * * * * * * * * * * * * * * * * * * * * * */
 
 
/* Includes */
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <signal.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#include "minesweeper.h"
//#include "message.h"
//#include "message.c"


/* Defines */
#define DEFAULT_PORT 12345
#define MAX_RX_SIZE 1000
#define MAX_TX_SIZE 50
#define MAX_NAME_LENGTH 20
static volatile bool terminate = false;

/* Fuction Prototypes */
bool rcvMsg(int cID, char* rxBuffer);
bool sndMsg(int cID, char* txBuffer);

/* Function declarations */
/// intHandler
/// On SIGINT, sets 'terminate' to true
void intHandler(int dummy)
{
	printf("%s", "\nSIGINT received!\n");
	terminate = true;
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

/// initGame
/// initialise game
void initGame(GameState* game){
	m_initGame(game);
	printf("\n\n\n\n\n");
	printf("Remaining mines: %d\n", game->remainingMines); // Display remaining mine count
	printf("\n    1 2 3 4 5 6 7 8 9\n");
	printf("-----------------------\n");
	for(int row = 0; row < 9; row++){
		char rowChar = 'A' + row;
		printf("%c | ", rowChar);
		
		for(int col = 0; col < 9; col++){
			if(getTile(game, row, col) == 0){
				printf("  ");
			}else{
				printf("! "); // Debug
			}
		}
		printf("\n");
	}
}

/// processGame
/// Reads user input and processes that information
void processGame(GameState* game, char* data){
	// get user input
	// check that it is a valid input
	// process user request
	// process server response
	
	bool flagNoMine = false;
	if (data != NULL) {
		// Process tiles
		int size_rx = strlen(data);
		int numTiles = ((size_rx - 11)/12) + 1;
		int x,y,n,f,m;
		for(int i = 0; i < numTiles; i++){
			// extract first cell of data into temp
			char tmp[11];
			for(int j = 0; j < 11; j++){
				tmp[j] = data[j];
			}
		
			// store data into gameState
			sscanf(tmp,"t,%d,%d,%d,%d,%d",&x,&y,&n,&f,&m);

			((game->tiles)[x][y]).nAdjacentMines = n;
			((game->tiles)[x][y]).isFlagged = f;
			((game->tiles)[x][y]).isMine = m;
			((game->tiles)[x][y]).isRevealed = true;

			// Decrement remaining mine count if Flagged & Mine
			if (f && m && !game->isOver) {
				--(game->remainingMines);
			}
			else if (f && !game->isOver) {
				// Flag placed but no mine
				flagNoMine = true;
			}
		
			// truncate data to remove first cell
			memmove(&data[0],&data[11], strlen(data) - 11);
		
			if(i < numTiles - 1){
				memmove(&data[0],&data[1], strlen(data) - 1);
			}
		}
	}

	// Display result
	printf("\n\n\n\n\n");
	if (flagNoMine) printf("Oops! No mine at previously placed flag!\n");
	printf("Remaining mines: %d\n", game->remainingMines); // Display remaining mine count
	printf("\n    1 2 3 4 5 6 7 8 9\n");
	printf("-----------------------\n");
	for(int row = 0; row < 9; row++){
		char rowChar = 'A' + row;
		printf("%c | ", rowChar);
		
		for(int col = 0; col < 9; col++){
			if(game->isOver && !game->isWon) {
				if(((game->tiles)[col][row]).isMine)
					printf("* ");
				else
					printf("  ");
			}else if(((game->tiles)[col][row]).isFlagged){
				printf("+ ");
			}else if(!((game->tiles)[col][row]).isRevealed){
				printf("  ");
			}else{
				printf("%d ", ((game->tiles)[col][row]).nAdjacentMines);
			}
		}
		printf("\n");
	}
}


/// Doubly linked list Entry for leaderboard
typedef struct Entry {
	char name[MAX_NAME_LENGTH];
	long int time;
	int wins, plays;
	struct Entry* prv;
	struct Entry* next;
} Entry;

/// leaderBoard
/// displays sorted leaderboard in descending order, fastest first!
void leaderBoard(char* data){
	// Header
	printf("========================================== LEADERBOARD ==========================================\n\n");

	// Error
	if (strncmp(data, "error", 5) == 0) {
		printf("Leaderboard is currently empty...\n");	
	}

	Entry* entries = NULL;
	Entry* lastEntry = NULL;
	int entryCount = 0;

	// Extract entries
	int dataLen = strlen(data);
	int ptrOffset = 0;
	while(ptrOffset < dataLen) {
		// Scan for correct args		
		Entry* entry = malloc(sizeof(Entry));
		entry->prv  = NULL;
		entry->next = NULL;
		int consumed;
		if( sscanf(data+ptrOffset, "l,%[^,\n],%ld,%d,%d%n", 
			       entry->name, &(entry->time), &(entry->wins), &(entry->plays), 
                   &consumed) < 4
          ){
			printf("Failed to extract leaderboard data\n\n");
			free(entry);
			break; // Could not extract data
		}

		// Offset
		ptrOffset += consumed + 1; // skip extra comma
		
		// Push to list
		if (entries == NULL) {
			entries = entry;
			lastEntry = entry;
		}
		else {
			entry->prv = lastEntry;
			lastEntry->next = entry;
			lastEntry = entry;
		}
		entryCount++;
	}

	// Display lb in descending order
	while (entryCount > 0) {
		long int highestTime = -1;
		Entry* highestEntry = NULL;
		Entry* entry = entries; // problematic if first entry is deleted

		// Find next worst entry
		while(entry != NULL) {
			// Store highest time
			if (entry->time > highestTime) {
				highestTime = entry->time;
				highestEntry = entry;
			}
			else if (entry->time == highestTime) {
				// Sort reverse-win count
				if (entry->wins < highestEntry->wins) {
					highestEntry = entry;
				}
				else if (entry->wins == highestEntry->wins) {
					// Sort reverse-alphabetically
					if (strcmp(entry->name, highestEntry->name) < 0)
						highestEntry = entry;
				}
			}
			entry = entry->next;
		}

		// Display next worst entry
		printf("%-20s %10ld seconds              %5d games won, %d games played\n",
			   highestEntry->name, highestEntry->time, highestEntry->wins, highestEntry->plays);

		// Pop worst entry
		if (highestEntry->prv != NULL)
			highestEntry->prv->next = highestEntry->next;
		else 
			entries = highestEntry->next; // Replace first list pointer to next entry
		if (highestEntry->next != NULL)
			highestEntry->next->prv = highestEntry->prv;
		free(highestEntry);
		--entryCount;
	}
	
	// Footer
	printf("\n=================================================================================================\n\n");

}


/// main
int main(int argc, char* argv[])
{
	// Setup signals
	struct sigaction sa;
	sa.sa_handler = intHandler;
	sa.sa_flags = 0;  // stop SA_RESTART interfering with quitting
	sigemptyset(&sa.sa_mask);
	sigaction(SIGINT, &sa, NULL);
	signal(SIGPIPE, SIG_IGN);

	// Get Host info
	struct hostent *he;
	if (argc > 1) {
		if ((he = gethostbyname(argv[1])) == NULL) {
			herror("gethostbyname");
			exit(1);
		}
	}
	else if ((he = gethostbyname("LOCALHOST")) == NULL) {
		herror("gethostbyname");
		exit(1);
	}

	// Check for a port numbeer
	int port = DEFAULT_PORT;
	if (argc > 2 && atoi(argv[2]) > 0) {
		port = atoi(argv[2]);
	}

	// Open socket
	printf("Opening socket on port %d...\n", port);
	int cID;
	if ((cID = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		perror("socket");
		exit(1);
	}

	struct sockaddr_in cAddr;
	cAddr.sin_family = AF_INET;      // Host byte order
	cAddr.sin_port = htons(port);    // Short, network byte order
	cAddr.sin_addr = *((struct in_addr *)he->h_addr);
	bzero(&(cAddr.sin_zero), 8);     // Zero the rest of the struct

	// Connect
	printf("Connecting to Server...\n");
	if (connect(cID, (struct sockaddr *)&cAddr, sizeof(struct sockaddr)) == -1) {
		perror("connect");
		exit(1);
	}

	// Receive and send messages
	char user[MAX_NAME_LENGTH], pass[MAX_NAME_LENGTH];
	char rxBuffer[MAX_RX_SIZE];
	char txBuffer[MAX_TX_SIZE];
	memset(rxBuffer, 0, sizeof(rxBuffer)/sizeof(char));
	memset(txBuffer, 0, sizeof(txBuffer)/sizeof(char));
	memset(user, 0, sizeof(user)/sizeof(char));
	memset(pass, 0, sizeof(pass)/sizeof(char));
	
	
	/* --- initialise the game and game state --- */
	GameState game;
	GameState* gamePtr = &game;
	
	bool auth = false;
	bool gameStart = false;
	
	while (!terminate) {
		
		/* --- Receive --- */
		if(!rcvMsg(cID,rxBuffer)){
			// Handle auth fail
			if (!auth) {
				printf("Failed to authenticate! Disconnected.\n\n");
			}
			break;
		}
		
		/* --- Check server response and update game state --- */
		// Main menu
		if (!gameStart) {
			if(strstr(txBuffer,"exit")){
				exit(1);
			}
			else if(strstr(txBuffer,"lb") != NULL){
				leaderBoard(rxBuffer);
			}
			else if(strstr(rxBuffer,"accept") != NULL){
				// Accept from authentication
				if(auth == false){
					auth = true;
				}
				// Accept from 'play'
				else {
					gameStart = true;
					initGame(gamePtr);
				}
			}
		}
		// Game
		else {
			// Tile revealed
			if(strstr(rxBuffer,"t,") != NULL){		
				processGame(gamePtr, rxBuffer);		
			}
			// Game over
			else if (strstr(rxBuffer, "over,") != NULL){
				if (rxBuffer[5] == '0') {
					// Game over
					game.isOver = true;
					gameStart = false;

					// Receive all tiles
					if(!sndMsg(cID, "ok"))
						break;
					if(!rcvMsg(cID,rxBuffer))
						break;

					processGame(gamePtr, rxBuffer);
					printf("\n\nGame over! You hit a mine!\n\n");
				}
				else {
					// Game won
					game.isOver = true;
					game.isWon  = true;
					game.remainingMines = 0;
					gameStart = false;
					processGame(gamePtr, NULL); // Display game with no update
					printf("\n\nCongratulations! You have located all the mines. You won in %s seconds!\n\n", rxBuffer+7);
				}
			}
			else {
				// Some unhandled error
			}
		}

		/* --- end server stuff --- */
		
		
		
		/* --- user input --- */
		if(!auth){
			printf("==============================\n");
			printf("WELCOME TO MINESWEEPER ONLINE!\n");
			printf("==============================\n\n");
			printf("Input enter your username and password in the format of 'user,pass':\n");
		}else if(!gameStart){
			printf("\nMain menu:\n");
			printf("Type 'play' to begin the game.\n");
			printf("Type 'lb' to see the leaderboard.\n");
			printf("Type 'exit' to quit program.\n");
		}else if(!game.isOver){
			printf("\nGame menu:\n");
			printf("Type 'r,<1-9>,<A-I>' to reveal a position.\n");
			printf("Type 'f,<1-9>,<A-I>' to flag a position.\n");
			printf("Type 'quit' to end game.\n");
		}
		/* --- End user input --- */
		
		
		
		/* --- Send --- */
		size_t txLen = MAX_TX_SIZE;
		char* b = txBuffer;

		// Ensure correct format
		bool formatOK = false;
		while(!formatOK) {
			// Extract a line from console
			if( getline(&b, &txLen, stdin) == -1) {
				printf("No line\n");
				if (terminate) break;
			}

			if (!gameStart) {
				// Authenticating
				if (!auth)
					formatOK = true;

				// Options are 'play', 'exit', 'lb'
				// Check string matches
				else if (strncmp(txBuffer, "play", 4) == 0)
					formatOK = true;
				else if (strncmp(txBuffer, "lb", 2) == 0)
					formatOK = true;
				else if (strncmp(txBuffer, "exit", 4) == 0)
					formatOK = true;
				else
					printf("%s", "Invalid option. Try again!\n");
			}
			else { 
				// Options are "r,<x>,<y>", "f,<x>,<y>", "quit", or "winhack"
				int x = 0; char y = 0;
				if (strncmp(txBuffer, "quit", 4) == 0)
					formatOK = true;
				else if (strncmp(txBuffer, "winhack", 7) == 0)
					formatOK = true;
				else if (strncmp(txBuffer, "r", 1) == 0 &&
						 sscanf(txBuffer, "r,%1d,%1c", &x, &y) == 2)
						formatOK = true;
				else if (strncmp(txBuffer, "f", 1) == 0 &&
						 sscanf(txBuffer, "f,%1d,%1c", &x, &y) == 2)
						formatOK = true;
				else 
					printf("%s", "Invalid option. Try again!\n");

				if (formatOK && x && y) {
					// Check bounds on x, y
					y = tolower(y);
					if (x < 1 || x > 9 || y < 'a' || y > 'i') {
						printf("Selection out of grid bounds. Try again!\n");
						formatOK = false;
					}
					// Make sure tile has not already been revealed
					else if( game.tiles[x-1][y-'a'].isRevealed ) {
						printf("Tile %d,%c has already been revealed.\n", x, y);
						formatOK = false;
					}
					// Make sure tile has not already been flagged
					else if( game.tiles[x-1][y-'a'].isFlagged ) {
						printf("Tile %d,%c has already been flagged.\n", x, y);
						formatOK = false;
					}
					// Convert to array format
					else {
						txBuffer[2] = txBuffer[2] - 1;
						if(txBuffer[4] >= 'a' && txBuffer[4] <= 'i'){
							txBuffer[4] = txBuffer[4] - 'a' + '1' - 1;
						}else if(txBuffer[4] >= 'A' && txBuffer[4] <= 'I'){
							txBuffer[4] = txBuffer[4] - 'A' + '1' - 1;
						}else{
							txBuffer[4] = txBuffer[4] - 1;
						}
					}
				}
			}
		} // End while: formatOK == true

		// Send message
		if(!sndMsg(cID, txBuffer))
			break; // Failed to send message
		/* --- End Send --- */
		
	}
	
	closeSocket(cID);
	return 0;
}

bool sndMsg(int cID, char* txBuffer){
	//printf("Sending !%s!\n", txBuffer);
	if (send(cID, txBuffer, strlen(txBuffer), 0) == -1) {
		perror("Failed to send data");
		return false;
	}
	return true;
}

bool rcvMsg(int cID, char* rxBuffer){
	memset(rxBuffer, 0, MAX_RX_SIZE*sizeof(char));

	if (recv(cID, rxBuffer, MAX_RX_SIZE, 0) <= 0) {
		perror("Failed to receive data");
		closeSocket(cID);
		return false;
	}
	//printf("Received: !%s!\n", rxBuffer);
	return true;
}

