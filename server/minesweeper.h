/* * * * * * *                                                            * * * * * * * * * * * * * * * * * * * * *
 * server/minesweeper.h
 * Header for server-side minesweeper game code
 *
 * Author:  Keagan Godfrey
 * Version: 1.0
 * Date:    2/10/2018
 * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef __server_minesweeper__h__
#define __server_minesweeper__h__

/* Includes */
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>


/* Defines */
#define N_TILES_X 9
#define N_TILES_Y 9
#define N_MINES   10
#define WARNING   -1
#define MINE_HIT  -2
#define FLAGGED_MINE   1


/* Types */
/// Tile structure
typedef struct
{
	int nAdjacentMines;
	bool isRevealed;
	bool isMine;
	bool isFlagged;
} Tile;


/// GameState structure
typedef struct
{
	bool isOver;
	bool isWon;
	int remainingMines;
	time_t startTime;
	time_t endTime;
	Tile tiles[N_TILES_X][N_TILES_Y];
} GameState;


/* Public function prototypes */
/// initGame
/// Sets up a new GameState structure, including mine placement
void initGame(GameState* game);


/// requestReveal
/// Requests a tile reveal
/// Assumes reply has been cleared with "memset(reply, 0, sizeof(reply)/sizeof(char))"
/// Assumes reply is large enough to host message for multiple tile reveals
int requestReveal(GameState* game, int x, int y, char* reply);


/// requestFlag
/// Requests a flag placement
/// Assumes reply has been cleared with "memset(reply, 0, sizeof(reply)/sizeof(char))"
int requestFlag(GameState* game, int x, int y, char* reply);


/// requestAllTiles
/// Requests every tile be revealed
/// Assumes reply has been cleared with "memset(reply, 0, sizeof(reply)/sizeof(char))"
/// Assumes reply is large enough to host message for multiple tile reveals
int requestAllTiles(GameState* game, char* reply);


/// forceWin
/// Triggers a game won response (hack, or play-testing)
void forceWin(GameState* game);

#endif
