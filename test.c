#include <gb/gb.h>
#include <stdio.h>
#include <time.h>
#include <gb/drawing.h>
#include <rand.h>
#include <types.h>

#include "tiledata.h"

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
  
  set_sprite_data(lastFreeTileValue, tileSum, gb_tile_data/*gso->tileDataPointer*/);
  // Refresh lastFreeTile value
  *(gso->lastFreeTilePointer) = lastFreeTileValue + tileSum; // replace +=
  // Link tiles and sprites
  for (i = 0; i < tileSum; i++) {
    set_sprite_tile(i, i);
  }
}

void newGso(GameSpriteObject *gsoPointer,
            UINT8 width, 
            UINT8 height,
            UINT8 *tileDataPointer,
            UINT8 *lastFreeTilePointer) {
  // Without this line height === 0 ¯\_(ツ)_/¯
  gsoPointer->height;

  gsoPointer->width = width;
  gsoPointer->height = height;
  gsoPointer->tileDataPointer = tileDataPointer;
  gsoPointer->firstTileNum = *lastFreeTilePointer;
  gsoPointer->lastFreeTilePointer = lastFreeTilePointer;

  initGso(gsoPointer);
}

void shift_sprite(UBYTE x, UBYTE y)
{
  UBYTE i, j, c = 0;

  for (i = 0; i < 3; ++i) {
    for (j = 0; j < 3; ++j) {
      move_sprite(c++, x + 8 * j, y + 8 * i);
    }
  }
}

void init_sprite(unsigned char *tile_data)
{
  UBYTE i;
  set_sprite_data(0, 9, tile_data);

  for (i = 0; i < 9; ++i) {
    set_sprite_tile(i, i);
  }
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

  UINT8 lastFreeTile = 0;

  UINT8 playerWidth = 3;
  UINT8 playerHeight = 3;
  UINT8 *playerTileDataPointer = &gb_tile_data;
  GameSpriteObject player;

  newGso(&player, playerWidth, playerHeight, playerTileDataPointer, &lastFreeTile);

  SPRITES_8x8;
  SHOW_BKG;
  SHOW_SPRITES;
  DISPLAY_ON;

  while(resume) {
    j = joypad();
    if (j & J_A && !delay) {
      yd = gh - y;
      time_backup = clock();
      delaying = clock() + JUMP_DELAY;
      delay = TRUE;
    }

    if (j & J_RIGHT) 
      x++;
    if (j & J_LEFT) 
      x--;
    if (clock() > delaying) 
      delay = FALSE;
    t = clock() - time_backup;
    dx0 = v0 * t / DOWNTEMPO_COEFFICIENT;
    dx1 = g * t * t / DOWNTEMPO_COEFFICIENT / 2;
    coord =  dx0 - dx1;
    vcoord = coord;
    y = yd + vcoord;
    y = gh - y;

    shift_sprite(x, y);
    if (y < 10 || y > GRAPHICS_HEIGHT) {
      resume = 0;
      printf("FAGGOT");
    }
  }
}
