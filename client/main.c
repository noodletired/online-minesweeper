/* * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * client/main.c
 * Minesweeper client main entrypoint
 *
 * Author:  Keagan Godfrey
 * Version: 1.0
 * Date:    24/10/2018
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
#include <netdb.h>


/* Defines */
#define DEFAULT_PORT 12345
#define MAX_RX_SIZE 1000
#define MAX_TX_SIZE 50
#define MAX_NAME_LENGTH 20
static volatile bool terminate = false;


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
	if (argc > 2 && atoi(argv[2]) < 1) {
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
	while (!terminate) {
		// Receive first
		memset(rxBuffer, 0, sizeof(rxBuffer)/sizeof(char));
		if (recv(cID, rxBuffer, MAX_RX_SIZE, 0) <= 0) {
			perror("Failed to receive data");
			closeSocket(cID);
			break;
		}
		printf("Received: %s\n", rxBuffer);
		
		// Send
		size_t txLen = MAX_TX_SIZE;
		char* b = txBuffer;
		if( getline(&b, &txLen, stdin) == -1) {
			printf("No line\n");
		}
		printf("Sending %s\n", txBuffer);
		if (send(cID, txBuffer, txLen, 0) == -1) {
			perror("Failed to send data");
			break;
		}
	}
	
	closeSocket(cID);
	return 0;
}
