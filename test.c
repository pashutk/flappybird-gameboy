#include <gb/gb.h>
#include <stdio.h>
#include <time.h>
#include <gb/drawing.h>
#include <rand.h>
#include <types.h>

#include "tiledata.h"

int abs(int num) {
  if(num < 0)
    return -num;
  else
    return num;
}


typedef struct GameSpriteObject_ {
  UBYTE width;
  UBYTE height;
  UBYTE *tileDataPointer;
  UBYTE firstTileNum;
  UBYTE *lastFreeTilePointer;
} GameSpriteObject;

void initGso(GameSpriteObject *gso) {
  UBYTE i;
  UBYTE tileSum;
  UBYTE lastFreeTileValue;

  lastFreeTileValue = *(gso->lastFreeTilePointer);
  tileSum = gso->width * gso->height;
  
  set_sprite_data(lastFreeTileValue, tileSum, gso->tileDataPointer);
  // Refresh lastFreeTile value
  *(gso->lastFreeTilePointer) += tileSum;
  // Link tiles and sprites
  for (i = lastFreeTileValue; i < lastFreeTileValue + tileSum; i++) {
    set_sprite_tile(i, i);
  }
}

void newGso(GameSpriteObject *gsoPointer,
            UINT8 width, 
            UINT8 height,
            UINT8 *tileDataPointer,
            UINT8 *lastFreeTilePointer) {
  // Without this lines some params === 0 ¯\_(ツ)_/¯
  gsoPointer->height;
  gsoPointer->tileDataPointer;

  gsoPointer->width = width;
  gsoPointer->height = height;
  gsoPointer->tileDataPointer = tileDataPointer;
  gsoPointer->firstTileNum = *lastFreeTilePointer;
  gsoPointer->lastFreeTilePointer = lastFreeTilePointer;

  initGso(gsoPointer);
}

// Work correctly with 8*8 sprites
void moveGso(GameSpriteObject *gso, UINT8 x, UINT8 y) {
  UBYTE i, j, c;
  c = gso->firstTileNum;

  for (i = 0; i < gso->height; ++i) {
    for (j = 0; j < gso->width; ++j) {
      move_sprite(c++, x + 8 * j, y + 8 * i);
    }
  }
}

// TODO: increase render speed by using line-by-line drawing (vertical)
void drawPipe(UINT8 x, UINT8 level) {
  const UINT8 pipeWidth = 5;
  UINT8 i, j;
  UINT8 randomBkgTiles[18 * 5];
  UINT8 tileNum = 0;
  UINT8 gap = 6;
  UINT8 capHeight = 3;

  for (j=0; j != 18; j++) {
    for (i=0; i != pipeWidth; i++) {
      if (j <= level - gap - capHeight || j > level + capHeight) {
        // Draw pipe body
        if (i == 0) {
          tileNum = 1;
        } else if (i == 1 || i == 2) {
          tileNum = 2;
        } else if (i == 3) {
          tileNum = 3;
        } else if (i == 4) {
          tileNum = 4;
        } else {
          tileNum = 0;
        }
      } else if (j <= level - gap && j > level - gap - capHeight) {
        // Draw top pipe cap
        if (j == level - gap - 2) {
          if (i == 0) {
            tileNum = 18;
          } else if (i == 1) {
            tileNum = 19;
          } else if (i == 2) {
            tileNum = 20;
          } else if (i == 3) {
            tileNum = 21;
          } else if (i == 4) {
            tileNum = 22;
          } else {
            tileNum = 0;
          }
        } else if (j == level - gap - 1) {
          if (i == 0) {
            tileNum = 23;
          } else if (i == 1) {
            tileNum = 24;
          } else if (i == 2) {
            tileNum = 25;
          } else if (i == 3) {
            tileNum = 25;
          } else if (i == 4) {
            tileNum = 26;
          } else {
            tileNum = 0;
          }
        } else if (j == level - gap) {
          if (i == 0) {
            tileNum = 27;
          } else if (i == 1) {
            tileNum = 28;
          } else if (i == 2) {
            tileNum = 28;
          } else if (i == 3) {
            tileNum = 29;
          } else if (i == 4) {
            tileNum = 30;
          } else {
            tileNum = 0;
          }
        }
      } else if (j > level && j <= level + capHeight) {
        // Draw bottom pipe cap 
        if (j == level + 1) {
          if (i == 0) {
            tileNum = 5;
          } else if (i == 3) {
            tileNum = 7;
          } else if (i == 4) {
            tileNum = 8;
          } else if (i == 1 || i == 2) {
            tileNum = 6;
          } else {
            tileNum = 0;
          }
        } else if (j == level + 2) {
          if (i == 0) {
            tileNum = 9;
          } else if (i == 3) {
            tileNum = 11;
          } else if (i == 4) {
            tileNum = 12;
          } else if (i == 1 || i == 2) {
            tileNum = 10;
          } else {
            tileNum = 0;
          }
        } else if (j == level + 3) {
          if (i == 0) {
            tileNum = 13;
          } else if (i == 1) {
            tileNum = 14;
          } else if (i == 2) {
            tileNum = 15;
          } else if (i == 3) {
            tileNum = 16;
          } else if (i == 4) {
            tileNum = 17;
          } else {
            tileNum = 0;
          }
        }
      } else {
        tileNum = 0;
      }
      randomBkgTiles[pipeWidth * j + i] = tileNum;
    }
  }
  set_bkg_tiles(x, 0, pipeWidth, 18, randomBkgTiles);
}

void flushRow(UINT8 row) {
  UINT8 i = 0;
  UINT8 randomBkgTiles[31];
  for (i = 0; i < 32; i++) {
    randomBkgTiles[i] = 0;
  }
  set_bkg_tiles(0, row, 31, 1, randomBkgTiles);
}

void flushBkg() {
  UINT8 i = 0;
  for (i = 0; i < 32; i++) {
    flushRow(i);
  }
}

int randBetween(UINT8 min, UINT8 max) {
  int r = (int) max - min;
  return (rand() % r) + (int) min;
}

void main()
{
  const JUMP_DELAY = 15;
  const DOWNTEMPO_COEFFICIENT = 70;
  const DWORD v0 = 200;
  UBYTE x = 50, j;
  DWORD y = 0;
  DWORD coord = 0, vcoord = 0, dx0 = 150, dx1 = 150;
  UBYTE yd = 50;
  DWORD time_backup = 0;
  DWORD g = 10, gh = 144;
  DWORD delaying = 0;
  UBYTE delay = FALSE;
  UBYTE resume = TRUE;
  UBYTE yi = 0;
  UBYTE t = 0, u = 0;
  UINT8 i = 0;

  UINT8 lastFreeTile = 0;
  UINT8 scrollPositionX = 0;
  UINT8 firstPipeRerenderPoint = 0;
  UINT8 secondPipeRerenderPoint = 64;

  UINT8 playerWidth = flbird_tile_map_width;
  UINT8 playerHeight = flbird_tile_map_height;
  UINT8 *playerTileDataPointer = &flbird_tile_data;
  GameSpriteObject player;

  newGso(&player, playerWidth, playerHeight, playerTileDataPointer, &lastFreeTile);

  set_bkg_data(1, flbody_tile_count, flbody_tile_data);
  set_bkg_data(5, flbottomtop_tile_count, flbottomtop_tile_data);
  set_bkg_data(18, fltopbottom_tile_count, fltopbottom_tile_data);

  flushBkg();

  drawPipe(10, randBetween(8, 12));
  drawPipe(27, randBetween(8, 12));

  SPRITES_8x8;
  SHOW_BKG;
  SHOW_SPRITES;
  DISPLAY_ON;
  
  while(resume) {

    move_bkg(scrollPositionX, 0);
    scrollPositionX += 3;

    if (scrollPositionX > firstPipeRerenderPoint && scrollPositionX < firstPipeRerenderPoint + 3) {
      drawPipe(27, randBetween(8, 12));
    }
    if (scrollPositionX > secondPipeRerenderPoint && scrollPositionX < secondPipeRerenderPoint + 3) {
      drawPipe(10, randBetween(8, 12));
    }

    
    j = joypad();
    if (j & J_A && !delay) {
      yd = gh - y;
      time_backup = clock();
      delaying = clock() + JUMP_DELAY;
      delay = TRUE;
    }

    if (j & J_RIGHT) 
      x+=3;
    if (j & J_LEFT) 
      x-=3;
    if (clock() > delaying) 
      delay = FALSE;
    t = clock() - time_backup;
    dx0 = v0 * t / DOWNTEMPO_COEFFICIENT;
    dx1 = g * t * t / DOWNTEMPO_COEFFICIENT / 2;
    coord =  dx0 - dx1;
    vcoord = coord;
    y = yd + vcoord;
    y = gh - y;

    moveGso(&player, x, y);
    if (y < 10 || y > GRAPHICS_HEIGHT) {
      resume = 0;
      // printf("FAGGOT");
    }
  }
}
