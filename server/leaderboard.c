/* * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * server/leaderboard.c
 * Minesweeper server leaderboard
 *
 * Author:  Keagan Godfrey
 * Version: 1.0
 * Date:    2/10/2018
 * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/* Includes */
#include <leaderboard.h>
#include <string.h>


/* Defines */
static UserRecord* userRecords = NULL;


/* Private functions */
/// newUser
/// Creates a new user and pushes to end of user records
UserRecord* newUser(const char* name)
{
	// Allocate memory
	UserRecord* user = malloc(sizeof(UserRecord));
	strcpy(user->name, name);
	user->records = NULL;
	user->wins = 0;
	user->plays = 0;
	user->next = NULL;
	
	// Add to userRecords list
	UserRecord* current = userRecords;
	while(current != NULL )
		current = current->next;
	
	current->next = user;
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
	
	// Stop here if loss
	if (!win)
		return;
	
	// Update user win count and push record to end
	user->wins++;
	
	WinRecord* latest = user->records;
	if (latest == NULL) {
		user->records = record;
		return;
	}
	while(latest->next != NULL)
		latest = latest->next;
	latest->next = record;
}


/* Public functions */
/// newRecord
/// Adds a new record to the end of the list
void newRecord(const char* name, bool win, time_t time)
{
	// First check if loss
	if (!win) {
		updateUser(name, win, NULL);
		return;
	}
	
	// Generate a new win record
	WinRecord* record = malloc(sizeof(WinRecord));
	record.time = time;
	record.next = NULL;
	
	updateUser(name, win, record);
}


/// requestLeaderboard
/// Requests the entire leaderboard in a message
int requestLeaderboard(char* reply)
{	
	int replyLen = 0;
	char buffer[MAX_NAME_LENGTH+10+5+5+7]; // l,<name>,<time:long>,<wins:int>,<plays:int>
	
	// Iterate every user record
	UserRecord* user = userRecords;
	while(user != NULL) {
		// Iterate every win record
		WinRecord* record = user->records;
		while(record != NULL) {
			// Format record
			memset(&buffer, 0, sizeof(buffer)/sizeof(char));
			sprintf(&buffer, "l,%s,%l,%d,%d,", user->name, record->time, user->wins, user->plays);
			replyLen += strlen(buffer);
			
			// Append record to the reply
			strcat(reply, &buffer);
			
			record = record->next;
		}
		user = user->next;
	}
	
	// Replace last , with 0
	reply[replyLen] = 0;
	
	return len; // return size of reply
}


/// cleanupLeaderboard
/// Safely deallocates entire list of user records, including win records
void cleanupLeaderboard()
{
	// Store pointers for the current user record and next 
	UserRecord* currentUser = NULL;
	UserRecord* nextUser = userData;
	
	// Repeatedly shift next into current, and free current
	while(nextUser != NULL) {
		currentUser = next;
		nextUser = current->next;
		
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