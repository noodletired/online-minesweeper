/* * * * * * *                                                            * * * * * * * * * * * * * * * * * * * * *
 * server/threadpool.h
 * Header for server-side threadpool code
 *
 * Author:  Keagan Godfrey
 * Version: 1.0
 * Date:    12/10/2018
 * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef __server_threadpool__h__
#define __server_threadpool__h__

/* Includes */
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>


/* Defines */
#define NUM_THREADS 10


/* Types */
/// Request structure
/// Linked list
struct Request_t {
    void (*callback)(int);
	int data;
    struct Request_t* next;
};
typedef struct Request_t Request;


/* Public function prototypes */
/// newRequest
/// Adds a request to the requests list
void newRequest(void (*callback)(int), int data);


/// initThreadpool
/// Initialises a pool of worker threads
void initThreadpool();


/// destroyThreadpool
/// Forces all threads to exit
void destroyThreadpool();


#endif