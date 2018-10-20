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
		
		// place mine at (x, y)
		game->tiles[x][y].isMine = true;
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
/// requestReveal
/// Requests a tile reveal
void requestReveal(GameState* game, int x, int y, char* reply)
{
	// Reveal tile
	int err = revealTile(game, x, y);
	
	// Mine hit!
	if (err == MINE_HIT){
		game->isOver = true;
		return; // game over
	}
	
	// Already revealed
	if (err == WARNING){
		return; // warn player the tile is already revealed
	}
	
	// Success
		// compose message of all newly revealed tiles
	
}


/// requestFlag
/// Requests a flag placement
void requestFlag(GameState* game, int x, int y, char* reply)
{
	// Place flag
	int err = placeFlag(game, x, y);
	
	// Tile revealed or flag aready placed
	if (err == WARNING){
		return; // warn player the tile is already revealed
	}
	
	// Check for successful flag on mine
	if (err == SUCCESS){
		--(game->remainingMines);
		
		// Notify correct flag placement
		
		// Win condition
		if (game->remainingMines == 0) {
			game->isOver = true;
			game->isWon  = true;
			// add game time to the message
			
			return;
		}
		
		return;
	}
		
	// Unsuccessful flag on mine
		// tell client no mine exists at location
	
}