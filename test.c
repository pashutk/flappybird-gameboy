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

  UINT8 playerWidth = flbird_tile_map_width;
  UINT8 playerHeight = flbird_tile_map_height;
  UINT8 *playerTileDataPointer = &flbird_tile_data;
  GameSpriteObject player;

  GameSpriteObject pipeBottom;
  GameSpriteObject pipeBody;

  newGso(&player, playerWidth, playerHeight, playerTileDataPointer, &lastFreeTile);

  newGso(&pipeBottom, fltopbottom_tile_map_width, fltopbottom_tile_map_height, fltopbottom_tile_data, &lastFreeTile);
  moveGso(&pipeBottom, 70, 56);

  // Sprite graphic have a restriction: only 40 tiles on screen.
  // Defenitely need to use background layer
  // set_bkg_data(0, flbody_tile_count, flbody_tile_data);
  // set_bkg_tiles()

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
      printf("FAGGOT");
    }
  }
}
