#define _GNU_SOURCE

/* * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * server/leaderboard.c
 * Minesweeper server leaderboard
 *
 * Author:  Keagan Godfrey
 * Version: 1.0
 * Date:    2/10/2018
 * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/* Includes */
#include "leaderboard.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>


/* Defines */
static UserRecord* userRecords = NULL;
static int readCounter = 0;
static pthread_mutex_t rcLock = PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP; // mutex lock for read counter
static pthread_mutex_t lbLock = PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP; // mutex lock for leaderboard access


/* Private functions */
/// newUser
/// Creates a new user and pushes to end of user records
UserRecord* newUser(const char* name)
{
	// Allocate memory for new user
	UserRecord* user = malloc(sizeof(UserRecord));
	if (!user) {
        perror("Out of memory in newUser");
        exit(1);
    }
	
	// Fill data
	strcpy(user->name, name);
	user->records = NULL;
	user->wins = 0;
	user->plays = 0;
	user->next = NULL;
	
	// Add to userRecords list
	UserRecord* current = userRecords;
	if (current == NULL) {
		userRecords = user;
	}
	else {
		while(current->next != NULL)
			current = current->next;
		current->next = user;
	}

	// Success
	printf("Successfully created new user.\n");
	fflush(stdout);

	return user;
}


/// updateUser
/// Adds or updates user data when a new play finishes
void updateUser(const char* name, bool win, WinRecord* record)
{
	// Check if user exists
	UserRecord* user = userRecords;
	bool userExists = false;
	while(user != NULL) {
		if (strncmp(name, user->name, MAX_NAME_LENGTH) == 0) {
			userExists = true;
			break;
		}
		
		user = user->next;
	}
	if (userExists == false)
		user = newUser(name);
	
	// Increment play count
	user->plays++;
	printf("Incremented playcount for %s\n", name);
	fflush(stdout);
	
	// Stop here if loss
	if (!win)
		return;
	
	// Update user win count and push record to end
	user->wins++;
	
	WinRecord* latest = user->records;
	if (latest == NULL) {
		user->records = record;
	}
	else {
		while(latest->next != NULL)
			latest = latest->next;
		latest->next = record;
	}

	// Success
	printf("Added new win record for %s\n", name);
	fflush(stdout);
}


/* Public functions */
/// newRecord
/// Adds a new record to the end of the list
void newRecord(const char* name, bool win, long int time)
{
	// Lock
	pthread_mutex_lock(&lbLock);
	
	// First check if loss
	if (!win) {
		updateUser(name, win, NULL);
		pthread_mutex_unlock(&lbLock); // remember to unlock the mutex!
		return;
	}
	
	// Allocate memory for new record
	WinRecord* record = malloc(sizeof(WinRecord));
	if (!record) {
		perror("Out of memory in newRecord");
		exit(1);
	}
	
	record->time = time;
	record->next = NULL;
	
	updateUser(name, win, record);
	
	// Unlock
	pthread_mutex_unlock(&lbLock);
}


/// requestLeaderboard
/// Requests the entire leaderboard in a message
int requestLeaderboard(char* reply)
{	
	// Wait for reader lock
	pthread_mutex_lock(&rcLock);
	readCounter++;
	
	// First reader must wait for and lock leaderboard, then unlock reader lock
	if (readCounter == 1)
		pthread_mutex_lock(&lbLock);
	pthread_mutex_unlock(&rcLock);
		
	
	int replyLen = 0;
	char buffer[MAX_NAME_LENGTH+10+5+5+7]; // l,<name>,<time:long>,<wins:int>,<plays:int>
	
	// Iterate every user record
	UserRecord* user = userRecords;
	while(user != NULL) {
		// Iterate every win record
		WinRecord* record = user->records;
		while(record != NULL) {
			// Format record
			memset(buffer, 0, sizeof(buffer)/sizeof(char));
			sprintf(buffer, "l,%s,%ld,%d,%d,", user->name, record->time, user->wins, user->plays);
			replyLen += strlen(buffer);
			
			// Append record to the reply
			strcat(reply, buffer);
			
			record = record->next;
		}
		user = user->next;
	}
	
	// Wait for reader lock
	pthread_mutex_lock(&rcLock);
	readCounter--;
	
	// Last reader must unlock leaderboard
	if (readCounter == 0)
		pthread_mutex_unlock(&lbLock);
	pthread_mutex_unlock(&rcLock);
	
	// Replace last , with 0
	reply[replyLen-1] = 0;
	
	return replyLen; // return size of reply
}


/// cleanupLeaderboard
/// Safely deallocates entire list of user records, including win records
void cleanupLeaderboard()
{
	// Store pointers for the current user record and next 
	UserRecord* currentUser = NULL;
	UserRecord* nextUser = userRecords;
	
	// Repeatedly shift next into current, and free current
	while(nextUser != NULL) {
		currentUser = nextUser;
		nextUser = currentUser->next;
		
		// Do the same with each win record
		WinRecord* currentWin = NULL;
		WinRecord* nextWin = currentUser->records;
		while(nextWin != NULL) {
			currentWin = nextWin;
			nextWin = currentWin->next;
			free(currentWin);
		}
		
		free(currentUser);
	}
}
