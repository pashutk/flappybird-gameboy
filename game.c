#include <gb/gb.h>
#include <stdio.h>
#include <time.h>
#include <gb/drawing.h>
#include <rand.h>
#include <types.h>

#include "tiledata.h"

const UINT8 pipeGap = 8;
const JUMP_DELAY = 15;

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
void drawPipe(UINT8 x, INT8 level) {
  const UINT8 pipeWidth = 5;
  const INT8 capHeight = 3;
  INT8 i, j;
  UINT8 pipeTiles[18 * 5];
  UINT8 tileNum = 0;

  // printf("%d\n", level);

  for (j = 0; j != 18; j++) {
    for (i = 0; i != pipeWidth; i++) {
      tileNum = 0;
      if (j + capHeight <= level || j > level + pipeGap + capHeight) {
        // Draw pipe body
        if (i == 0) {
          tileNum = 1;
        } else if (i == 1 || i == 2) {
          tileNum = 2;
        } else if (i == 3) {
          tileNum = 3;
        } else if (i == 4) {
          tileNum = 4;
        }
        // printf("b%d\n", (UWORD)j);
      } else if (j <= level && j > level - capHeight) {
        // Draw top pipe cap
        // printf("t%d\n", (UWORD)capHeight);
        if (j + 2 == level) {
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
          }
        } else if (j + 1 == level) {
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
          }

        } else if (j == level) {
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
          }
        }
      } else if (j > level + pipeGap && j <= level + pipeGap + capHeight) {
        // Draw bottom pipe cap
        // printf("l%d\n", (UWORD)j);
        if (j == level + pipeGap + 1) {
          if (i == 0) {
            tileNum = 5;
          } else if (i == 3) {
            tileNum = 7;
          } else if (i == 4) {
            tileNum = 8;
          } else if (i == 1 || i == 2) {
            tileNum = 6;
          }
        } else if (j == level + pipeGap + 2) {
          if (i == 0) {
            tileNum = 9;
          } else if (i == 3) {
            tileNum = 11;
          } else if (i == 4) {
            tileNum = 12;
          } else if (i == 1 || i == 2) {
            tileNum = 10;
          }
        } else if (j == level + pipeGap + 3) {
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
          }
        }
      }
      pipeTiles[pipeWidth * j + i] = tileNum;
    }
  }
  set_bkg_tiles(x, 0, pipeWidth, 18, pipeTiles);
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

// int pipeRandomLevel( UINT8 max) {
//   int r = (int) max - min;
//   return (rand() % r) + (int) min;
// }

UBYTE pipeRandomLevel() {
  const UBYTE max = 10;
  UBYTE num;
  num = rand() % max;
  while (num > 200) {
    num = rand() % max;
  }
  return num;
}

BOOLEAN checkCollision(DWORD y) {
  if (y < 10 || y > GRAPHICS_HEIGHT) {
    return FALSE;
  } else {
    return TRUE;
  }
}

BOOLEAN checkPipeCollision(DWORD y, INT8 level) {
  if (y < level * 8 + flbird_tile_map_height * 8 || y > level * 8 + pipeGap * 8) {
    return TRUE;
  } else {
    return FALSE;
  }
}

enum GameState {
  TUTORIAL,
  MAIN,
  FAIL
};

INT16 getPlayerYPos(UINT8 t, UINT8 yd) {
  const INT8 DOWNTEMPO_COEFFICIENT = 70;
  const INT32 v0 = 200;
  const INT8 g = 10;
  INT32 dx0, dx1;

  dx0 = v0 * t / DOWNTEMPO_COEFFICIENT;
  dx1 = g * t * t / DOWNTEMPO_COEFFICIENT / 2;
  return GRAPHICS_HEIGHT - (yd + (dx0 - dx1));
}



void main() {
  UBYTE playerXPosition = 50, j;
  DWORD y = 0;
  UBYTE yd = 50;
  UINT16 time_backup = 0, delaying = 0;
  BOOLEAN delay = FALSE, resume = TRUE;
  UBYTE yi = 0;
  UINT8 i = 0, t = 0, u = 0;

  BOOLEAN needToCheckPipeCollision = FALSE;

  UINT8 lastFreeTile = 0;
  UWORD scrollPositionX = 0;
  UWORD startFirstPipeCollisionX;
  UWORD endFirstPipeCollisionX;
  UWORD startSecondPipeCollisionX;
  UWORD endSecondPipeCollisionX;
  const UINT8 firstPipeRerenderPoint = 0;
  const UINT8 secondPipeRerenderPoint = 128;
  const UINT8 screenScrollShift = 4;
  UINT8 randomLevel1;
  UINT8 randomLevel2;
  BOOLEAN resumeGame = TRUE;
  const UINT8 initialTutorialArrowPosition = 176;
  UINT8 tutorialArrowPosition = initialTutorialArrowPosition;
  const UINT8 tutorialArrowShownPosition = 120;
  const int arrowVerticalPosition = 68;

  enum GameState gameState = TUTORIAL;
  GameSpriteObject player;
  GameSpriteObject arrow;
  GameSpriteObject aButton;

  newGso(&player, flbird_tile_map_width, flbird_tile_map_height, &flbird_tile_data, &lastFreeTile);
  newGso(&arrow, arrow_tile_map_width, arrow_tile_map_height, &arrow_tile_data, &lastFreeTile);
  newGso(&aButton, ab_tile_map_width, ab_tile_map_height, &ab_tile_data, &lastFreeTile);

  set_bkg_data(1, flbody_tile_count, flbody_tile_data);
  set_bkg_data(5, flbottomtop_tile_count, flbottomtop_tile_data);
  set_bkg_data(18, fltopbottom_tile_count, fltopbottom_tile_data);

  flushBkg();

  SPRITES_8x8;
  SHOW_BKG;
  SHOW_SPRITES;
  DISPLAY_ON;

  startFirstPipeCollisionX = 10 * 8 - (playerXPosition + (3 * 4));
  endFirstPipeCollisionX = startFirstPipeCollisionX + 40 + 2 * 8;
  startSecondPipeCollisionX = 255 - 40 - playerXPosition - 3 * 4;
  endSecondPipeCollisionX = startSecondPipeCollisionX + 40 + 2 * 8;

  moveGso(&arrow, tutorialArrowPosition, arrowVerticalPosition);
  moveGso(&aButton, 168, 80);

  playerXPosition = 0;
  y = 144-50;

  while(1) {
    // Main loop
    j = joypad();

    if (gameState == TUTORIAL) {
      if (tutorialArrowPosition > tutorialArrowShownPosition) {
        tutorialArrowPosition -= 4;
        moveGso(&arrow, tutorialArrowPosition, arrowVerticalPosition);
        moveGso(&aButton, tutorialArrowPosition - 3, 80);
      }

      if (playerXPosition < 50) {
        playerXPosition += 4;
        moveGso(&player, playerXPosition, y);
      }

      if (j & J_A && !delay) {
        time_backup = clock();
        delaying = clock() + JUMP_DELAY;
        delay = TRUE;
        gameState = MAIN;
      }
    }

    // Game loop
    if (gameState == MAIN) {
      if (tutorialArrowPosition < initialTutorialArrowPosition) {
        tutorialArrowPosition += 4;
        moveGso(&arrow, tutorialArrowPosition, arrowVerticalPosition);
        moveGso(&aButton, tutorialArrowPosition - 3, 80);
      }

      move_bkg(scrollPositionX, 0);
      if (scrollPositionX < 255 - screenScrollShift) {
        scrollPositionX += screenScrollShift;
      } else {
        scrollPositionX = 255 - scrollPositionX;
      }

      if (scrollPositionX > firstPipeRerenderPoint && scrollPositionX <= firstPipeRerenderPoint + screenScrollShift) {
        randomLevel1 = pipeRandomLevel();
        drawPipe(27, randomLevel1);
      }
      if (scrollPositionX > secondPipeRerenderPoint && scrollPositionX <= secondPipeRerenderPoint + screenScrollShift) {
        randomLevel2 = pipeRandomLevel();
        drawPipe(10, randomLevel2);
        needToCheckPipeCollision = TRUE;
      }

      if (j & J_A && !delay) {
        yd = GRAPHICS_HEIGHT - y;
        time_backup = clock();
        delaying = clock() + JUMP_DELAY;
        delay = TRUE;
      }

      if (clock() > delaying) {
        delay = FALSE;
      }
      t = clock() - time_backup;
      y = getPlayerYPos(t, yd);

      moveGso(&player, playerXPosition, y);

      if (needToCheckPipeCollision == FALSE) {
        resume = checkCollision(y);
      } else if (scrollPositionX >= startFirstPipeCollisionX && scrollPositionX <= endFirstPipeCollisionX) {
        if (checkPipeCollision(y, randomLevel2) == TRUE) {
          resume = FALSE;
        }
      } else if (scrollPositionX >= startSecondPipeCollisionX && scrollPositionX <= endSecondPipeCollisionX) {
        if (checkPipeCollision(y, randomLevel1) == TRUE) {
          resume = FALSE;
        }
      } else {
        resume = checkCollision(y);
      }

      if (resume == FALSE) {
        gameState = FAIL;
        yd = GRAPHICS_HEIGHT - y;
        time_backup = clock();
      }
    }

    if (gameState == FAIL) {
      // printf("sdf");
      t = clock() - time_backup;
      y = getPlayerYPos(t, yd);

      moveGso(&player, playerXPosition, y);
    }

    wait_vbl_done();
  }
}
