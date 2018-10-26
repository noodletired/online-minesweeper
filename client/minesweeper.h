/* * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * client/minesweeper.h
 * Minesweeper client main entrypoint
 *
 * Author:  Christopher Dare
 * Version: 1.0
 * Date:    24/10/2018
 * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef __client_minesweeper__h__
#define __client_minesweeper__h__

/* Includes */
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>


/* Defines */
#define N_TILES_X	9
#define N_TILES_Y	9
#define N_MINES		10
#define FLAG		-1
#define MINE		-10
#define FLAGGED_MINE	-2

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


/* --- Public Function Prototypes --- */

// initGame
// sets up the new game structure
void m_initGame(GameState* game);

// getTile
// returns the Tile value at location x,y
int getTile(GameState* game, int x, int y);








#endif
