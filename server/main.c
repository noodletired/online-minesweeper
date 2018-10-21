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


/* Defines */
#define SEED 42
#define DEFAULT_PORT 12345
#define BACKLOG 10
static volatile bool terminate = false;


/* Function declarations */
/// intHandler
/// On SIGINT, sets 'terminate' to true
void intHandler(int dummy)
{
	printf("%s", "\nSIGINT received!\n");
	terminate = true;
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


/// openSocket
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
	
	printf("Server disconnected safely...\n");
}


/// authUser
int authUser()
{
	return 0;
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
	//initThreadpool(NUM_THREADS);
	
	// Set port from args
	int port = DEFAULT_PORT;
	if (argc > 1 && atoi(argv[1]) < 1) {
		port = atoi(argv[1]);
	}
	
	// Open socket
	socklen_t sInSize = sizeof(struct sockaddr_in);
	int sID = openSocket(port);
	
	// Run loop
	while(!terminate) {
		// Block until new client connection
		struct sockaddr_in cAddr;
		int cID = accept(sID, (struct sockaddr*)&cAddr, &sInSize);
		if (cID == -1) {
			perror("Failed to accept connection");
			continue;
		}
		printf("Server accepted connection from %s\n", inet_ntoa(cAddr.sin_addr));
		
		// Add request for threaded processing
	}

	// Clean up
	closeSocket(sID);
	
	return 0;
}
 
