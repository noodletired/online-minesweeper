/* * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * server/main.c
 * Minesweeper server main entrypoint
 *
 * Author:  Keagan Godfrey
 * Version: 1.0
 * Date:    2/10/2018
 * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/* Includes */
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <signal.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "leaderboard.h"
#include "threadpool.h"
#include "comms.h"


/* Defines */
#define SEED 42
#define DEFAULT_PORT 12345
static volatile bool terminate = false;


/* Function declarations */
/// intHandler
/// On SIGINT, sets 'terminate' to true
void intHandler(int dummy)
{
	printf("%s", "\nSIGINT received!\n");
	terminate = true;
}


/// main
int main(int argc, char* argv[])
{
	// Seed random number generator
	srand(SEED);
	
	// Setup signals
	struct sigaction sa;
	sa.sa_handler = intHandler;
	sa.sa_flags = 0;  // stop SA_RESTART interfering with quitting
	sigemptyset(&sa.sa_mask);
	sigaction(SIGINT, &sa, NULL);
	signal(SIGPIPE, SIG_IGN);
	
	// Initialise threadpool
	initThreadpool();
	
	// Set port from args
	int port = DEFAULT_PORT;
	if (argc > 1 && atoi(argv[1]) > 0) {
		port = atoi(argv[1]);
	}
	
	// Open socket
	socklen_t sInSize = sizeof(struct sockaddr_in);
	int sID = openSocket(port);
	
	// Run loop
	while (!terminate) {
		// Block until new client connection
		struct sockaddr_in cAddr;
		int cID = accept(sID, (struct sockaddr*)&cAddr, &sInSize);
		if (cID == -1) {
			perror("Failed to accept connection");
			continue;
		}
		printf("Server accepted connection from %s\n", inet_ntoa(cAddr.sin_addr));
		
		// Add request for threaded processing
		newRequest(handleConnection, cID);
	}

	// Clean up
	closeSocket(sID);
	destroyThreadpool();
	cleanupLeaderboard();
	printf("Server exited safely.\n");
	
	return 0;
}
 
