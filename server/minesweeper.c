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


/* Private functions */
/// placeMines
/// Randomly sets N_MINES game tiles to be mines
void placeMines(GameState* game)
{
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
		if( y > 0 )
			game->tiles[x][y-1].nAdjacentMines++;
		
		// northeast
		if( y > 0 && x < N_TILES_X-1 )
			game->tiles[x+1][y-1].nAdjacentMines++;
		
		// east
		if( x < N_TILES_X-1 )
			game->tiles[x+1][y].nAdjacentMines++;
		
		// southeast
		if( x < N_TILES_X-1 && y < N_TILES_Y-1 )
			game->tiles[x+1][y+1].nAdjacentMines++;
		
		// south
		if( y < N_TILES_Y-1 )
			game->tiles[x][y+1].nAdjacentMines++;
		
		// southwest
		if( y < N_TILES_Y-1 && x > 0 )
			game->tiles[x-1][y+1].nAdjacentMines++;
		
		// west
		if( x > 0 )
			game->tiles[x-1][y].nAdjacentMines++;
		
		// northwest
		if( x > 0 && y > 0 )
			game->tiles[x-1][y-1].nAdjacentMines++;
	}
}


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


/// tileAdjacentMines
/// Assumes (x < N_TILES_X) && (y < N_TILES_Y)
/// Returns the number of adjacent mines to tile at (x, y)
int tileAdjacentMines(GameState* game, int x, int y)
{
	return game->tiles[x][y].nAdjacentMines;
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
		if( y > 0 )
			revealTile(game, x, y-1);
		
		// northeast
		if( y > 0 && x < N_TILES_X-1 )
			revealTile(game, x+1, y-1);
		
		// east
		if( x < N_TILES_X-1 )
			revealTile(game, x+1, y);
		
		// southeast
		if( x < N_TILES_X-1 && y < N_TILES_Y-1 )
			revealTile(game, x+1, y+1);
		
		// south
		if( y < N_TILES_Y-1 )
			revealTile(game, x, y+1);
		
		// southwest
		if( y < N_TILES_Y-1 && x > 0 )
			revealTile(game, x-1, y+1);
		
		// west
		if( x > 0 )
			revealTile(game, x-1, y);
		
		// northwest
		if( x > 0 && y > 0 )
			revealTile(game, x-1, y-1);
	}
	
	return SUCCESS; // No error
}


/* Public functions */
/// initGame
/// Sets up a new GameState structure, including mine placement
void initGame(GameState* game)
{
	// Set defaults
	game->isOver = false;
	gmae->isWon = false;
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
	int err = revealTile(game, x, y, changes);
	
	// Mine hit!
	if (err == MINE_HIT){
		game->isOver = true;
		game->endTime = time(0);
		return; // game over
	}
	
	// Already revealed
	if (err == WARNING){
		sprintf(reply, "error");
		return;
	}
	
	// Compose message of all newly revealed tiles
	int replyLen = 0;
	char buffer[13]; // t,<x>,<y>,<n>,<flagged>,<mine>,
	for (int i=0; i<N_TILES_X; i++) {
		for (int j=0; j<N_TILES_Y; j++) {
			if (!tileIsRevealed(oldGame,i,j) && tileIsRevealed(game,i,j)) {
				// Format tile data
				memset(&buffer, 0, sizeof(buffer)/sizeof(char));
				sprintf(&buffer, "t,%d,%d,%d,%d,%d,", i, j, tileAdjacentMines(game,i,j), tileIsFlagged(game,i,j), tileIsMine(game,i,j));
				replyLen += strlen(buffer);
				
				// Append to the reply
				strcat(reply, &buffer);
			}
		}
	}
	
	// Replace last , with 0
	reply[replyLen] = 0;
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
		return; // warn player the tile is already revealed
	}
	
	// Check for successful flag on mine
	if (err == SUCCESS){
		--(game->remainingMines);
		
		// Win condition
		if (game->remainingMines == 0) {
			game->isOver = true;
			game->isWon  = true;
			game->endTime = time(0);
			return;
		}
	}

	// Compose message indicating flagged tile
	sprintf(reply, "t,%d,%d,-1,1,%d", x, y, tileIsMine(game, x, y));
	return strlen(reply);
}