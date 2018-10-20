/* * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * server/leaderboard.h
 * Header for server-side minesweeper game code
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


/* Defines */
#define MAX_NAME_LENGTH 20


/* Types */
/// WinRecord structure
/// Linked list recording time to win
struct WinRecord_t
{
	time_t time;
	struct WinRecord_t* next;
};
typedef WinRecord_t WinRecord;


/// UserRecord structure
/// Linked list recording player name, win count, plays
struct UserRecord_t
{
	char[MAX_NAME_LENGTH] name;
	int plays;
	int wins;
	WinRecord* records;
	struct UserRecord_t* next;
};
typedef UserRecord_t UserRecord;


/* Public function prototypes */
/// newRecord
/// Adds a new user record
void newRecord(const char* name, bool win, time_t time);


/// requestLeaderboard
/// Requests the entire leaderboard in a message
int requestLeaderboard(char* reply);


/// cleanupLeaderboard
/// Safely deallocates entire list of user records, including win records
void cleanupLeaderboard();