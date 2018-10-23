#define _GNU_SOURCE

/* * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * server/minesweeper.c
 * Server-side minesweeper game code
 *
 * Author:  Keagan Godfrey
 * Version: 1.0
 * Date:    2/10/2018
 * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/* Includes */
#include "minesweeper.h"
#include <string.h>
#include <stdio.h>
#include <pthread.h>


/* Defines */
static pthread_mutex_t randLock = PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP; // mutex lock for rand() calls


/* Private functions */
/// tileIsMine
/// Assumes (x < N_TILES_X) && (y < N_TILES_Y)
/// Returns whether or not the game tile at (x, y) is a mine
bool tileIsMine(GameState* game, int x, int y)
{
	return game->tiles[x][y].isMine;
}


/// tileIsRevealed
/// Assumes (x < N_TILES_X) && (y < N_TILES_Y)
/// Returns whether or not the game tile at (x, y) is revealed
bool tileIsRevealed(GameState* game, int x, int y)
{
	return game->tiles[x][y].isRevealed;
}


/// tileIsFlagged
/// Assumes (x < N_TILES_X) && (y < N_TILES_Y)
/// Returns whether or not the game tile at (x, y) is revealed
bool tileIsFlagged(GameState* game, int x, int y)
{
	return game->tiles[x][y].isFlagged;
}


/// tileAdjacentMines
/// Assumes (x < N_TILES_X) && (y < N_TILES_Y)
/// Returns the number of adjacent mines to tile at (x, y)
int tileAdjacentMines(GameState* game, int x, int y)
{
	return game->tiles[x][y].nAdjacentMines;
}


/// placeMines
/// Randomly sets N_MINES game tiles to be mines
/// Requires synchronisation, since rand() is not thread safe
void placeMines(GameState* game)
{
	// Lock
    int err = pthread_mutex_lock(&randLock);
	
	for (int i=0; i<N_MINES; i++) {
		int x, y;
		do
		{
			x = rand() % N_TILES_X;
			y = rand() % N_TILES_Y;
		} while (tileIsMine(game, x, y));
		
		// Place mine at (x, y)
		game->tiles[x][y].isMine = true;
		
		// Increment nAdjacentMines on 8 neighbours
		// north
		if ( y > 0 )
			game->tiles[x][y-1].nAdjacentMines++;
		
		// northeast
		if ( y > 0 && x < N_TILES_X-1 )
			game->tiles[x+1][y-1].nAdjacentMines++;
		
		// east
		if ( x < N_TILES_X-1 )
			game->tiles[x+1][y].nAdjacentMines++;
		
		// southeast
		if ( x < N_TILES_X-1 && y < N_TILES_Y-1 )
			game->tiles[x+1][y+1].nAdjacentMines++;
		
		// south
		if ( y < N_TILES_Y-1 )
			game->tiles[x][y+1].nAdjacentMines++;
		
		// southwest
		if ( y < N_TILES_Y-1 && x > 0 )
			game->tiles[x-1][y+1].nAdjacentMines++;
		
		// west
		if ( x > 0 )
			game->tiles[x-1][y].nAdjacentMines++;
		
		// northwest
		if ( x > 0 && y > 0 )
			game->tiles[x-1][y-1].nAdjacentMines++;
	}
	
	// Unlock
    err = pthread_mutex_unlock(&randLock);
}


/// revealTile
/// Assumes (x < N_TILES_X) && (y < N_TILES_Y)
/// Sets selected tile to revealed
/// Recursively reveals tiles if nAdjacentMines == 0
int revealTile(GameState* game, int x, int y)
{
	// Check for mine
	if (tileIsMine(game, x, y))
		return MINE_HIT;
	
	// Skip revealed tiles
	if (tileIsRevealed(game, x, y))
		return WARNING;
	
	// Reveal tile
	game->tiles[x][y].isRevealed = true;
	
	// Recursively check 8-neighbours if no adjacent mines
	// Ignore return code
	if (game->tiles[x][y].nAdjacentMines == 0){
		// north
		if ( y > 0 )
			revealTile(game, x, y-1);
		
		// northeast
		if ( y > 0 && x < N_TILES_X-1 )
			revealTile(game, x+1, y-1);
		
		// east
		if ( x < N_TILES_X-1 )
			revealTile(game, x+1, y);
		
		// southeast
		if ( x < N_TILES_X-1 && y < N_TILES_Y-1 )
			revealTile(game, x+1, y+1);
		
		// south
		if ( y < N_TILES_Y-1 )
			revealTile(game, x, y+1);
		
		// southwest
		if ( y < N_TILES_Y-1 && x > 0 )
			revealTile(game, x-1, y+1);
		
		// west
		if ( x > 0 )
			revealTile(game, x-1, y);
		
		// northwest
		if ( x > 0 && y > 0 )
			revealTile(game, x-1, y-1);
	}
	
	// No error
	return 0;
}


/// placeFlag
/// Places flag at the specified position
int placeFlag(GameState* game, int x, int y)
{
	// Check for flagged or revealed
	if (tileIsFlagged(game, x, y) || tileIsRevealed(game, x, y))
		return WARNING;
	
	// Flag tile
	game->tiles[x][y].isFlagged = true;
	
	// Check for a successful mine coverage
	if (tileIsMine(game, x, y))
		return FLAGGED_MINE;
	
	// Flag placed but no hit
	return 0;
}



/* Public functions */
/// initGame
/// Sets up a new GameState structure, including mine placement
void initGame(GameState* game)
{
	// Set defaults
	game->isOver = false;
	game->isWon = false;
	game->remainingMines = N_MINES;
	game->startTime = time(0);
	game->endTime = 0;
	
	// Set default tiles
	for (int i=0; i<N_TILES_X; i++) {
		for (int j=0; j<N_TILES_Y; j++) {
			game->tiles[i][j] = (Tile){0, false, false, false}; //c99 shorthand
		}
	}
	
	// Place mines
	placeMines(game);
}


/// requestReveal
/// Requests a tile reveal
/// Assumes reply has been cleared with "memset(reply, 0, sizeof(reply)/sizeof(char))"
/// Assumes reply is large enough to host message for multiple tile reveals
int requestReveal(GameState* game, int x, int y, char* reply)
{
	// Clone GameState
	GameState oldGame = *game;
	
	// Reveal tile
	int err = revealTile(game, x, y);
	
	// Mine hit!
	if (err == MINE_HIT){
		game->isOver = true;
		game->endTime = time(0);
		return 0; // game over
	}
	
	// Already revealed
	if (err == WARNING){
		sprintf(reply, "error");
		return 6;
	}
	
	// Compose message of all newly revealed tiles
	int replyLen = 0;
	char buffer[13]; // t,<x>,<y>,<n>,<flagged>,<mine>,
	for (int i=0; i<N_TILES_X; i++) {
		for (int j=0; j<N_TILES_Y; j++) {
			if (!tileIsRevealed(&oldGame,i,j) && tileIsRevealed(game,i,j)) {
				// Format tile data
				memset(buffer, 0, sizeof(buffer)/sizeof(char));
				sprintf(buffer, "t,%d,%d,%d,%d,%d,", i, j, tileAdjacentMines(game,i,j), tileIsFlagged(game,i,j), tileIsMine(game,i,j));
				replyLen += strlen(buffer);
				
				// Append to the reply
				strcat(reply, buffer);
			}
		}
	}
	
	// Replace last , with 0
	reply[replyLen-1] = 0;

	return replyLen;
}


/// requestFlag
/// Requests a flag placement
/// Assumes reply has been cleared with "memset(reply, 0, sizeof(reply)/sizeof(char))"
int requestFlag(GameState* game, int x, int y, char* reply)
{
	// Place flag
	int err = placeFlag(game, x, y);
	
	// Tile revealed or flag aready placed
	if (err == WARNING){
		sprintf(reply, "error");
		return 6; // warn player the tile is already revealed
	}
	
	// Check for successful flag on mine
	if (err == FLAGGED_MINE){
		--(game->remainingMines);
		
		// Win condition
		if (game->remainingMines == 0) {
			game->isOver = true;
			game->isWon  = true;
			game->endTime = time(0);
			return 0;
		}
	}

	// Compose message indicating flagged tile
	sprintf(reply, "t,%d,%d,-1,1,%d", x, y, tileIsMine(game, x, y));
	return strlen(reply);
}


/// requestAllTiles
/// Requests every tile be revealed
/// Assumes reply has been cleared with "memset(reply, 0, sizeof(reply)/sizeof(char))"
/// Assumes reply is large enough to host message for multiple tile reveals
int requestAllTiles(GameState* game, char* reply)
{
	// Compose message of all revealed tiles
	int replyLen = 0;
	char buffer[13]; // t,<x>,<y>,<n>,<flagged>,<mine>,
	for (int i=0; i<N_TILES_X; i++) {
		for (int j=0; j<N_TILES_Y; j++) {
			// Format tile data
			memset(buffer, 0, sizeof(buffer)/sizeof(char));
			sprintf(buffer, "t,%d,%d,%d,%d,%d,", i, j, tileAdjacentMines(game,i,j), tileIsFlagged(game,i,j), tileIsMine(game,i,j));
			replyLen += strlen(buffer);
			
			// Append to the reply
			strcat(reply, buffer);
		}
	}
	
	// Replace last , with 0
	reply[replyLen-1] = 0;

	return replyLen;
}
