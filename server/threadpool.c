/* * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * server/threadpool.c
 * Minesweeper server main entrypoint
 *
 * Adapted from CAB403 Practical Code
 * Version: 1.0
 * Date:    12/10/2018
 * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/* Includes */
#include "threadpool.h"
#include <stdlib.h> 
#include <pthread.h>


/* Private data */
static pthread_t pool[NUM_THREADS];
static int thrID[NUM_THREADS];


/* Private functions */


/* Public functions */
/// initThreadpool
/// Initialises a pool of n worker threads
void initThreadpool(int numThreads, pool_t* pool, void (*f)(int))
{
	// Create the request-handling threads
	for (int i=0; i<numThreads; i++) {
		pthread_create(&pool[i], NULL, handleRequests, (void*)&thrID[i]);
	}
}


void destroyThreadpool()
{
	// Destroy each worker thread
}