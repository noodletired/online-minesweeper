/* * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * client/minesweeper.c
 * Minesweeper client main entrypoint
 *
 * Author:  Christopher Dare
 * Version: 1.0
 * Date:    24/10/2018
 * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/* Includes */
#include "minesweeper.h"
#include <string.h>
#include <stdio.h>
#include <pthread.h>



/* --- Public Functions --- */

void m_initGame(GameState* game){
	// Set defaults
	game->isOver = false;
	game->isWon = false;
	game->remainingMines = N_MINES;
	game->startTime = time(0);
	
	// Set default tiles
	for (int i=0; i<N_TILES_X; i++) {
		for (int j=0; j<N_TILES_Y; j++) {
			game->tiles[i][j] = (Tile){0, false, false, false}; //c99 shorthand
		}
	}
}

int getTile(GameState* game, int x, int y){
	return (((game->tiles)[x][y]).nAdjacentMines);
}
