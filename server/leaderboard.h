/* * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * server/leaderboard.h
 * Header for server-side minesweeper leaderboard
 *
 * Author:  Keagan Godfrey
 * Version: 1.0
 * Date:    2/10/2018
 * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef __server_leaderboard__h__
#define __server_leaderboard__h__

/* Includes */
#include <stdbool.h>
#include <time.h>
#include "comms.h" // for MAX_NAME_LENGTH


/* Types */
/// WinRecord structure
/// Linked list recording time to win
typedef struct WinRecord
{
	long int time;
	struct WinRecord* next;
} WinRecord;


/// UserRecord structure
/// Linked list recording player name, win count, plays
typedef struct UserRecord
{
	char name[MAX_NAME_LENGTH];
	int plays;
	int wins;
	WinRecord* records;
	struct UserRecord* next;
} UserRecord;


/* Public function prototypes */
/// newRecord
/// Adds a new user record
void newRecord(const char* name, bool win, long int time);


/// requestLeaderboard
/// Requests the entire leaderboard in a message
int requestLeaderboard(char* reply);


/// cleanupLeaderboard
/// Safely deallocates entire list of user records, including win records
void cleanupLeaderboard();


#endif
