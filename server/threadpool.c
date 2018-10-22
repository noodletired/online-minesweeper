/* * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * server/threadpool.c
 * Minesweeper server main entrypoint
 *
 * Author:  Keagan Godfrey
 *          Adapted from CAB403 Practical Code
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

static pthread_mutex_t reqLock = PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP;
static pthread_cond_t reqReady = PTHREAD_COND_INITIALIZER;
static int nRequests = 0;
struct Request* requests = NULL;
struct Request* lastRequest = NULL;


/* Private functions */
/// getRequest
/// Gets the first pending request from the requests list
Request* getRequest()
{
    int err; // error variable for pthread functions
    Request* request = NULL;

    // Lock reqLock
    err = pthread_mutex_lock(reqLock);

    if (nRequests > 0) {
        request = requests;
        requests = a_request->next;
		
		// This was the last request
        if (requests == NULL) {
            last_request = NULL;
        }
		
        num_requests--;
    }

    // Unlock reqLock
    err = pthread_mutex_unlock(reqLock);

    return request;
}


/// handleRequests
/// A function used by threads to pull new requests as they are made
void* handleRequests(void* data)
{
    int err;
    int threadId = *((int*)data);
    Request* request;

    // Lock reqLock
    err = pthread_mutex_lock(reqLock);

    while(1) {
        if (numRequests > 0) {
            request = getRequest();
            if (request) {
                // Notify we are handling request
				printf("Thread %d handling request...\n", threadId);
				
				// Unlock reqLock while we handle ours
                err = pthread_mutex_unlock(&reqLock);
				
				// Run request callback, then free request
                (*request->callback)(request->data);
                free(request);
				
				// Notify thread is finished with request
				printf("Thread %d finished with request...\n", threadId);
				
				// Check for thread cancellation
				pthread_testcancel();
				
                // Lock the mutex again
                err = pthread_mutex_lock(&reqLock);
            }
        }
        else {
			// Wait for request to become available
			err = pthread_cond_wait(&reqReady, &reqLock);
        }
    }
}


/* Public functions */
/// newRequest
/// Adds a request to the requests list
void newRequest(void (*callback)(int), int data)
{
    int err; // error variable for pthread functions

    // Allocate memory for request
    Request* request = malloc(sizeof(Request));
	if (!request) {
        perror("Out of memory in newRequest");
        exit(1);
    }
    request->callback = callback;
	request->data = data;
    request->next = NULL;

    // Lock reqLock
    err = pthread_mutex_lock(reqLock);

    // Add new request to end of list
    if (nRequests == 0) { // list is empty
        requests = request;
        lastRequest = request;
    }
    else {
        lastRequest->next = request;
        lastRequest = request;
    }

    // Increment total number of pending requests
    nRequests++;

    // Unlock reqLock
    err = pthread_mutex_unlock(reqLock);

    // Signal the condition variable
    err = pthread_cond_signal(reqReady);
}


/// initThreadpool
/// Initialises a pool of worker threads
void initThreadpool()
{
	// Create the request-handling threads
	for (int i=0; i<NUM_THREADS; i++) {
        thrID[i] = i;
		pthread_create(&pool[i], NULL, handleRequests, (void*)&thrID[i]);
	}
}


/// destroyThreadpool
/// Forces all threads to exit
void destroyThreadpool()
{
	// Destroy each worker thread
	for (int i=0; i<NUM_THREADS; i++) {
		pthread_cancel(pool[i]);
	}
}