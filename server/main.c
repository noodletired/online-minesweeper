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
#include <stdbool.h>
#include <signal.h>


/* Defines */
#define SEED 42
static volatile bool terminate = false;


/* Function declarations */
/// intHandler
/// On SIGINT, sets 'terminate' to true
void intHandler(int dummy) {
    terminate = true;
}


/// main
int main(int argc, char* argv[])
{
	// Seed random number generator
	srand(SEED);
	
	// Setup signals
	signal(SIGINT, intHandler);
	
	// Run loop
	while(!terminate) {
		// Send tile data
		
		// Await request
		
		// Process request
		
		// Send response
	}

	// Clean up
	return 0;
}
 
