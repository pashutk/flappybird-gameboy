#include <gb/gb.h>
#include <stdio.h>
#include <time.h>
#include <gb/drawing.h>
#include <rand.h>
#include <types.h>
#include <gb/font.h>

#include "tiledata.h"

#define PIPE_GAP 8
#define JUMP_DELAY 15
#define SPRITE_HEIGHT 8
#define PIPE_WIDTH 5 // flbody_tile_map_width
#define DOWNTEMPO_COEFFICIENT 70

#define FIRST_PIPE_POSITION_X 0
#define SECOND_PIPE_POSITION_X 128
#define GAME_BKG_SCROLL_STEP 4
#define FAIL_STEP_DELAY 400
#define TUTORIAL_ARROW_INITIAL_POSITION_Y 176
#define TUTORIAL_ARROW_SHOWN_POSITION_Y 120

INT16 abs(INT16 num) {
  if(num < 0)
    return -num;
  else
    return num;
}

typedef struct game_sprite_object_ {
  UINT8 width;
  UINT8 height;
  UINT8 *tile_data_pointer;
  UINT8 first_tile_num;
  UINT8 *last_free_tile_pointer;
} game_sprite_object;

enum game_states {
  TRANSITION_TO_TITLE,
  TITLE,
  TRANSITION_TO_TUTORIAL,
  TUTORIAL,
  MAIN,
  FAIL,
  TRANSITION_TO_RETRY
};

enum moving_out_state {
  SIDE,
  DOWN,
  DONE
};

void init_gso(game_sprite_object *gso) {
  UINT8 i;
  UINT8 tile_sum;
  UINT8 last_free_tile_value;

  last_free_tile_value = *(gso->last_free_tile_pointer);
  tile_sum = gso->width * gso->height;

  set_sprite_data(last_free_tile_value, tile_sum, gso->tile_data_pointer);
  // Refresh last_free_tile value
  *(gso->last_free_tile_pointer) += tile_sum;
  // Link tiles and sprites
  for (i = last_free_tile_value; i < last_free_tile_value + tile_sum; i++) {
    set_sprite_tile(i, i);
  }
}

void new_gso(game_sprite_object *gso_pointer,
             UINT8              width,
             UINT8              height,
             UINT8              *tile_data_pointer,
             UINT8              *last_free_tile_pointer) {
  // Without this lines some params === 0 ¯\_(ツ)_/¯
  gso_pointer->height;
  gso_pointer->tile_data_pointer;

  gso_pointer->width = width;
  gso_pointer->height = height;
  gso_pointer->tile_data_pointer = tile_data_pointer;
  gso_pointer->first_tile_num = *last_free_tile_pointer;
  gso_pointer->last_free_tile_pointer = last_free_tile_pointer;

  init_gso(gso_pointer);
}

// Work correctly with 8*8 sprites
void move_gso(game_sprite_object *gso,
              UINT8              x,
              UINT8              y) {
  UINT8 i, j, c;
  c = gso->first_tile_num;

  for (i = 0; i < gso->height; ++i) {
    for (j = 0; j < gso->width; ++j) {
      move_sprite(c++, x + 8 * j, y + 8 * i);
    }
  }
}

// TODO: increase render speed by using line-by-line drawing (vertical)
void draw_pipe(UINT8 x, INT8 level) {
  const INT8 cap_height = fltopbottom_tile_map_height;
  INT8 i, j;
  UINT8 pipe_tiles[GRAPHICS_HEIGHT / SPRITE_HEIGHT * PIPE_WIDTH];
  UINT8 tile_num = 0;

  for (j = 0; j != 18; j++) {
    for (i = 0; i != PIPE_WIDTH; i++) {
      tile_num = 0;
      if (j + cap_height <= level || j > level + PIPE_GAP + cap_height) {
        // Draw pipe body
        if (i == 0) {
          tile_num = 1;
        } else if (i == 1 || i == 2) {
          tile_num = 2;
        } else if (i == 3) {
          tile_num = 3;
        } else if (i == 4) {
          tile_num = 4;
        }
      } else if (j <= level && j > level - cap_height) {
        // Draw top pipe cap
        if (j + 2 == level) {
          if (i == 0) {
            tile_num = 18;
          } else if (i == 1) {
            tile_num = 19;
          } else if (i == 2) {
            tile_num = 20;
          } else if (i == 3) {
            tile_num = 21;
          } else if (i == 4) {
            tile_num = 22;
          }
        } else if (j + 1 == level) {
          if (i == 0) {
            tile_num = 23;
          } else if (i == 1) {
            tile_num = 24;
          } else if (i == 2) {
            tile_num = 25;
          } else if (i == 3) {
            tile_num = 25;
          } else if (i == 4) {
            tile_num = 26;
          }

        } else if (j == level) {
          if (i == 0) {
            tile_num = 27;
          } else if (i == 1) {
            tile_num = 28;
          } else if (i == 2) {
            tile_num = 28;
          } else if (i == 3) {
            tile_num = 29;
          } else if (i == 4) {
            tile_num = 30;
          }
        }
      } else if (j > level + PIPE_GAP && j <= level + PIPE_GAP + cap_height) {
        // Draw bottom pipe cap
        if (j == level + PIPE_GAP + 1) {
          if (i == 0) {
            tile_num = 5;
          } else if (i == 3) {
            tile_num = 7;
          } else if (i == 4) {
            tile_num = 8;
          } else if (i == 1 || i == 2) {
            tile_num = 6;
          }
        } else if (j == level + PIPE_GAP + 2) {
          if (i == 0) {
            tile_num = 9;
          } else if (i == 3) {
            tile_num = 11;
          } else if (i == 4) {
            tile_num = 12;
          } else if (i == 1 || i == 2) {
            tile_num = 10;
          }
        } else if (j == level + PIPE_GAP + 3) {
          if (i == 0) {
            tile_num = 13;
          } else if (i == 1) {
            tile_num = 14;
          } else if (i == 2) {
            tile_num = 15;
          } else if (i == 3) {
            tile_num = 16;
          } else if (i == 4) {
            tile_num = 17;
          }
        }
      }
      pipe_tiles[PIPE_WIDTH * j + i] = tile_num;
    }
  }
  set_bkg_tiles(x, 0, PIPE_WIDTH, 18, pipe_tiles);
}

void draw_title() {
  UINT8 i;
  UINT8 tiles[30];
  UINT8 tiles2[11];

  tiles2[0] = 65;
  tiles2[1] = 66;
  tiles2[2] = 67;
  tiles2[3] = 68;
  tiles2[4] = 68;
  tiles2[5] = 0;
  tiles2[6] = 0;
  tiles2[7] = 68;
  tiles2[8] = 69;
  tiles2[9] = 70;
  tiles2[10] = 66;
  tiles2[11] = 69;

  set_bkg_tiles(20, 31, 12, 1, tiles2);

  for (i = 0; i < 30; i++) {
    tiles[i] = i + 35;
  }
  set_bkg_tiles(21, 25, 10, 3, tiles);
}

void draw_land() {
  UINT8 i;
  UINT8 tiles[32];

  for (i = 0; i < 32; i+=2) {
    tiles[i] = 33;
    tiles[i+1] = 34;
  }
  set_bkg_tiles(0, 17, 32, 1, tiles);
}

void flush_row(UINT8 row) {
  UINT8 i = 0;
  UINT8 bkg_tiles[31];
  for (i = 0; i < 32; i++) {
    bkg_tiles[i] = 0;
  }
  set_bkg_tiles(0, row, 31, 1, bkg_tiles);
}

void flush_bkg() {
  UINT8 i = 0;
  for (i = 0; i < 32; i++) {
    flush_row(i);
  }
}

UINT8 get_random_pipe_level() {
  const UINT8 max = 9;
  UINT8 num = rand() % max;
  // exclude values < 0
  while (num > 200) {
    num = rand() % max;
  }
  return num;
}

BOOLEAN check_collision(DWORD y) {
  if (y < 10 || y > GRAPHICS_HEIGHT - SPRITE_HEIGHT) {
    return TRUE;
  } else {
    return FALSE;
  }
}

BOOLEAN check_bottom_collision(DWORD y) {
  if (y > GRAPHICS_HEIGHT) {
    return TRUE;
  } else {
    return FALSE;
  }
}

BOOLEAN check_pipe_collision(DWORD y, INT8 level) {
  const UINT8 player_height = flbird_tile_map_height;
  if (y < level * SPRITE_HEIGHT + player_height * SPRITE_HEIGHT ||
      y > level * SPRITE_HEIGHT + PIPE_GAP * SPRITE_HEIGHT) {
    return TRUE;
  } else {
    return FALSE;
  }
}

INT16 get_player_y_pos(UINT8 t, UINT8 yd) {
  const INT32 v0 = 200;
  const INT8 g = 10;
  INT32 dx0, dx1;

  dx0 = v0 * t / DOWNTEMPO_COEFFICIENT;
  dx1 = g * t * t / DOWNTEMPO_COEFFICIENT / 2;
  return GRAPHICS_HEIGHT - (yd + (dx0 - dx1));
}

BOOLEAN move_title_in() {
  const UINT8 end_point = 128;
  const UINT8 downtempo = 6;
  static INT16 title_position_x = 0;
  INT8 title_buffer;

  if (title_position_x <= end_point) {
    title_buffer = (end_point - title_position_x) / downtempo;
    if (title_buffer < 1) {
      title_buffer = 1;
    }
    title_position_x += title_buffer;
    move_bkg(title_position_x, 160);
    return FALSE;
  } else {
    return TRUE;
  }
}

BOOLEAN move_title_out() {
  static enum moving_out_state current_state = SIDE;
  static UINT32 bkg_position_x = 128;
  static UINT16 bkg_position_y = 160;
  static UINT32 counter = 0;
  UINT32 delta = 0;
  const UINT8 downtempo = 50;

  if (current_state == DONE) {
    return TRUE;
  }

  counter++;
  delta = (counter * counter) / downtempo;

  if (current_state == SIDE) {
    bkg_position_x += delta;
    if (bkg_position_x > 255) {
      bkg_position_x = 0;
      counter = 0;
      current_state = DOWN;
    }
  } else if (current_state == DOWN) {
    bkg_position_y += delta;
    if (bkg_position_y > 255) {
      bkg_position_y = 0;
      current_state = DONE;
    }
  }
  move_bkg(bkg_position_x, bkg_position_y);
  return FALSE;
}

BOOLEAN blackify_back() {
  static INT8 step = 0;
  UINT8 i;
  UINT8 tiles[20];
  UINT8 tile_num;

  delay(100);

  move_bkg(0, 0);

  switch (step) {
    case 0:
      tile_num = 71;
      break;
    case 1:
      tile_num = 72;
      break;
    case 2:
      tile_num = 73;
      break;
    case 3:
      tile_num = 74;
      break;
    case 4:
      tile_num = 75;
      break;
    case 5:
      return TRUE;
  }

  for (i = 0; i < 20; i++) {
    tiles[i] = tile_num;
  }
  for (i = 0; i < 18; i++) {
    set_bkg_tiles(0, i, 20, 1, tiles);
  }

  step++;

  return FALSE;
}

void main() {
  UINT8 player_position_x = 50, j;
  DWORD player_position_y = 0;
  UINT8 yd = 50;
  UINT16 time_backup = 0, delaying = 0;
  BOOLEAN jump_is_delayed = FALSE, resume = TRUE;
  UINT8 i = 0, t = 0, u = 0;
  BOOLEAN pipe_collision_check_is_needed = FALSE;

  UINT8 last_free_tile = 0;
  UINT16 scroll_position_x = 0;
  UINT16 first_pipe_collision_zone_start_x;
  UINT16 first_pipe_collision_zone_end_x;
  UINT16 second_pipe_collision_zone_start_x;
  UINT16 second_pipe_collision_zone_end_x;
  UINT8 second_pipe_current_level;
  UINT8 first_pipe_current_level;
  UINT8 tutorial_arrow_position_x = TUTORIAL_ARROW_INITIAL_POSITION_Y;
  UINT16 tutorial_arrow_position_y = 68;
  BOOLEAN was_collision_flag = FALSE;
  INT8 tutorial_step_counter = 0;

  enum game_states current_game_state = TRANSITION_TO_TITLE;
  game_sprite_object player;
  game_sprite_object tutorial_arrow;
  game_sprite_object tutorial_a_button;

  new_gso(&player, flbird_tile_map_width, flbird_tile_map_height, &flbird_tile_data, &last_free_tile);
  new_gso(&tutorial_arrow, arrow_tile_map_width, arrow_tile_map_height, &arrow_tile_data, &last_free_tile);
  new_gso(&tutorial_a_button, ab_tile_map_width, ab_tile_map_height, &ab_tile_data, &last_free_tile);

  set_bkg_data(1, flbody_tile_count, flbody_tile_data);
  set_bkg_data(5, flbottomtop_tile_count, flbottomtop_tile_data);
  set_bkg_data(18, fltopbottom_tile_count, fltopbottom_tile_data);
  set_bkg_data(33, land_tile_count, land_tile_data);
  set_bkg_data(34, land2_tile_count, land2_tile_data);
  set_bkg_data(35, title_tile_count, title_tile_data);

  set_bkg_data(65, p_tile_count, p_tile_data);
  set_bkg_data(66, r_tile_count, r_tile_data);
  set_bkg_data(67, e_tile_count, e_tile_data);
  set_bkg_data(68, s_tile_count, s_tile_data);
  set_bkg_data(69, t_tile_count, t_tile_data);
  set_bkg_data(70, a_tile_count, a_tile_data);
  set_bkg_data(71, onedot_tile_count, onedot_tile_data);
  set_bkg_data(72, twodots_tile_count, twodots_tile_data);
  set_bkg_data(73, threedots_tile_count, threedots_tile_data);
  set_bkg_data(74, fourdots_tile_count, fourdots_tile_data);
  set_bkg_data(75, black_tile_count, black_tile_data);


  flush_bkg();

  SPRITES_8x8;
  SHOW_BKG;
  SHOW_SPRITES;
  DISPLAY_ON;

  draw_land();

  draw_title();
  move_bkg(0, 160);

  first_pipe_collision_zone_start_x = 10 * 8 - (player_position_x + (3 * 4));
  first_pipe_collision_zone_end_x = first_pipe_collision_zone_start_x + 40 + 2 * 8;
  second_pipe_collision_zone_start_x = 255 - 40 - player_position_x - 3 * 4;
  second_pipe_collision_zone_end_x = second_pipe_collision_zone_start_x + 40 + 2 * 8;

  move_gso(&tutorial_arrow, tutorial_arrow_position_x, tutorial_arrow_position_y);
  move_gso(&tutorial_a_button, 168, 80);

  player_position_x = 0;
  player_position_y = 144-50;

  while(resume) {
    // Main loop

    // TODO: one pressing – one jump
    j = joypad();

    if (current_game_state == TRANSITION_TO_TITLE) {
      if (move_title_in() == TRUE) {
        current_game_state = TITLE;
      };
    }

    if (current_game_state == TITLE) {
      if (j & J_START) {
        initrand(DIV_REG);
        current_game_state = TRANSITION_TO_TUTORIAL;
      }
    }

    if (current_game_state == TRANSITION_TO_TUTORIAL) {
      if (move_title_out() == TRUE) {
        current_game_state = TUTORIAL;
      };
    }

    if (current_game_state == TUTORIAL) {
      if (tutorial_arrow_position_x > TUTORIAL_ARROW_SHOWN_POSITION_Y) {
        tutorial_arrow_position_x -= 4;
        move_gso(&tutorial_a_button, tutorial_arrow_position_x - 3, 80);
      }

      if (tutorial_step_counter == 8) {
        tutorial_arrow_position_y++;
      } if (tutorial_step_counter == 16) {
        tutorial_arrow_position_y--;
        tutorial_step_counter = 0;
      } else {
        tutorial_step_counter++;
      }

      move_gso(&tutorial_arrow, tutorial_arrow_position_x, tutorial_arrow_position_y);
      move_bkg(scroll_position_x++, 0);

      if (player_position_x < 50) {
        player_position_x += 4;
        move_gso(&player, player_position_x, player_position_y);
      }

      if (j & J_A && !jump_is_delayed) {
        time_backup = clock();
        delaying = clock() + JUMP_DELAY;
        jump_is_delayed = TRUE;
        scroll_position_x = 0;
        current_game_state = MAIN;
      }
    }

    if (current_game_state == MAIN) {
      if (tutorial_arrow_position_x < TUTORIAL_ARROW_INITIAL_POSITION_Y) {
        tutorial_arrow_position_x += 4;
        move_gso(&tutorial_arrow, tutorial_arrow_position_x, tutorial_arrow_position_y);
        move_gso(&tutorial_a_button, tutorial_arrow_position_x - 3, 80);
      }

      move_bkg(scroll_position_x, 0);
      if (scroll_position_x < 255 - GAME_BKG_SCROLL_STEP) {
        scroll_position_x += GAME_BKG_SCROLL_STEP;
      } else {
        scroll_position_x = 255 - scroll_position_x;
      }

      if (scroll_position_x > FIRST_PIPE_POSITION_X && scroll_position_x <= FIRST_PIPE_POSITION_X + GAME_BKG_SCROLL_STEP) {
        second_pipe_current_level = get_random_pipe_level();
        draw_pipe(27, second_pipe_current_level);
      }
      if (scroll_position_x > SECOND_PIPE_POSITION_X && scroll_position_x <= SECOND_PIPE_POSITION_X + GAME_BKG_SCROLL_STEP) {
        first_pipe_current_level = get_random_pipe_level();
        draw_pipe(10, first_pipe_current_level);
        pipe_collision_check_is_needed = TRUE;
      }

      if (j & J_A && !jump_is_delayed) {
        yd = GRAPHICS_HEIGHT - player_position_y;
        time_backup = clock();
        delaying = clock() + JUMP_DELAY;
        jump_is_delayed = TRUE;
      }

      if (clock() > delaying) {
        jump_is_delayed = FALSE;
      }
      t = clock() - time_backup;
      player_position_y = get_player_y_pos(t, yd);

      move_gso(&player, player_position_x, player_position_y);

      if (pipe_collision_check_is_needed == FALSE) {
        was_collision_flag = check_collision(player_position_y);
      } else if (scroll_position_x >= first_pipe_collision_zone_start_x && scroll_position_x <= first_pipe_collision_zone_end_x) {
        if (check_pipe_collision(player_position_y, first_pipe_current_level) == TRUE) {
          was_collision_flag = TRUE;
        }
      } else if (scroll_position_x >= second_pipe_collision_zone_start_x && scroll_position_x <= second_pipe_collision_zone_end_x) {
        if (check_pipe_collision(player_position_y, second_pipe_current_level) == TRUE) {
          was_collision_flag = TRUE;
        }
      } else {
        was_collision_flag = check_collision(player_position_y);
      }

      if (was_collision_flag == TRUE) {
        delay(FAIL_STEP_DELAY);
        current_game_state = FAIL;
        if (player_position_y > GRAPHICS_HEIGHT) {
          yd = 0;
        } else {
          yd = GRAPHICS_HEIGHT - player_position_y;
        }
        time_backup = clock();
      }
    }

    if (current_game_state == FAIL) {
      t = clock() - time_backup;
      player_position_y = get_player_y_pos(t, yd);

      move_gso(&player, player_position_x, player_position_y);
      if (check_bottom_collision(player_position_y) == TRUE) {
        current_game_state = TRANSITION_TO_RETRY;
      }
    }

    if (current_game_state == TRANSITION_TO_RETRY) {
      move_gso(&player, 200, 200);
      if (blackify_back() == TRUE) {
        
      }
    }

    wait_vbl_done();
  }
}
