/* * * * * * * * * * * * * * * * * * * * * * * * * * * *
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
#include "../shared/message.h"


/* Defines */
#define N_TILES_X 9
#define N_TILES_Y 9
#define N_MINES   10
#define WARNING   -1
#define ERROR     -2
#define SUCCESS   0


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
/// Create a GameState 

/// Reveal Tile

/// Place Flag


#endif