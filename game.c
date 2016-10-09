#include <stdio.h>
#include <time.h>
#include <gb/drawing.h>
#include <rand.h>
#include <types.h>
#include <gb/font.h>

#include "tiledata.h"

#define NON_VOLATILE_MEMORY_ADDRESS 0xAF00
#define PIPE_GAP 8
#define JUMP_DELAY 15
#define SPRITE_HEIGHT 8
#define PIPE_WIDTH 5 // flbody_tile_map_width
#define DOWNTEMPO_COEFFICIENT 70

#define FIRST_PIPE_POSITION_X 0
#define SECOND_PIPE_POSITION_X 128
#define GAME_BKG_SCROLL_STEP 2
#define FAIL_STEP_DELAY 400
#define TUTORIAL_ARROW_INITIAL_POSITION_Y 176
#define TUTORIAL_ARROW_SHOWN_POSITION_Y 120

UINT8 *RAMPtr;
UINT16 high_score_backup = 0;

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
  TRANSITION_TO_RETRY,
  RETRY
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

// Returns tile number for 0-9
UINT8 get_tile_num_for_number(UINT8 number) {
  return 76 + number;
}

void fill_pipe_row_with_numbers(UINT8 *row, UINT8 row_number, UINT16 number) {
  const UINT8 first_order = number % 10;
  row[row_number + 3] = get_tile_num_for_number(first_order);
  if (number >= 10) {
    row[row_number + 2] = get_tile_num_for_number(((number - first_order) % 100) / 10);
  }
  if (number >= 100) {
    row[row_number + 1] = get_tile_num_for_number(((number - first_order) % 1000) / 100);
  }
}

// TODO: increase render speed by using line-by-line drawing (vertical)
void draw_pipe(UINT8 x, INT8 level, UINT16 pipe_num) {
  const INT8 cap_height = fltopbottom_tile_map_height;
  const INT8 middle_level = GRAPHICS_HEIGHT / SPRITE_HEIGHT / 2 / 2;
  UINT8 opt_shift = 0;
  INT8 j;
  UINT8 pipe_tiles[GRAPHICS_HEIGHT / SPRITE_HEIGHT * PIPE_WIDTH];
  UINT8 tile_num = 0;
  UINT8 row_num;

  if (x == 27) {
    opt_shift = 1;
  }

  for (j = 0; j != 18; j++) {
    row_num = PIPE_WIDTH * j;
    if (j + cap_height <= level || j > level + PIPE_GAP + cap_height) {
      // Draw pipe body
      pipe_tiles[row_num] = 1;
      pipe_tiles[row_num + 1] = pipe_tiles[row_num + 2] = 2;
      pipe_tiles[row_num + 3] = 3;
      pipe_tiles[row_num + 4] = 4;
    } else if (j <= level && j > level - cap_height) {
      // Draw top pipe cap
      if (j + 2 == level) {
        pipe_tiles[row_num] = tile_num = 18;
        pipe_tiles[row_num + 1] = tile_num = 19;
        pipe_tiles[row_num + 2] = tile_num = 20;
        pipe_tiles[row_num + 3] = tile_num = 21;
        pipe_tiles[row_num + 4] = tile_num = 22;
      } else if (j + 1 == level) {
        pipe_tiles[row_num] = 23;
        pipe_tiles[row_num + 1] = 24;
        pipe_tiles[row_num + 2] = 25;
        pipe_tiles[row_num + 3] = 25;
        pipe_tiles[row_num + 4] = 26;
        if (level > middle_level) {
          fill_pipe_row_with_numbers(pipe_tiles, row_num, pipe_num);
        }
      } else if (j == level) {
        pipe_tiles[row_num] = 27;
        pipe_tiles[row_num + 1] = pipe_tiles[row_num + 2] = 28;
        pipe_tiles[row_num + 3] = 29;
        pipe_tiles[row_num + 4] = 30;
      }
    } else if (j > level + PIPE_GAP && j <= level + PIPE_GAP + cap_height) {
      // Draw bottom pipe cap
      if (j == level + PIPE_GAP + 1) {
        pipe_tiles[row_num] = 5;
        pipe_tiles[row_num + 1] = pipe_tiles[row_num + 2] = 6;
        pipe_tiles[row_num + 3] = 7;
        pipe_tiles[row_num + 4] = 8;
      } else if (j == level + PIPE_GAP + 2) {
        pipe_tiles[row_num] = 9;
        pipe_tiles[row_num + 1] = pipe_tiles[row_num + 2] = 10;
        pipe_tiles[row_num + 3] = 11;
        pipe_tiles[row_num + 4] = 12;
        if (level <= middle_level) {
          fill_pipe_row_with_numbers(pipe_tiles, row_num, pipe_num);
        }
      } else if (j == level + PIPE_GAP + 3) {
        pipe_tiles[row_num] = 13;
        pipe_tiles[row_num + 1] = 14;
        pipe_tiles[row_num + 2] = 15;
        pipe_tiles[row_num + 3] = 16;
        pipe_tiles[row_num + 4] = 17;
      }
    } else {
      if (j == 11) {
        pipe_tiles[row_num] = 90 + opt_shift;
        pipe_tiles[row_num + 1] = 91 + opt_shift;
        pipe_tiles[row_num + 2] = 92 + opt_shift;
        pipe_tiles[row_num + 3] = 93 + opt_shift;
        pipe_tiles[row_num + 4] = 94 + opt_shift;
      } else if (j == 12) {
        pipe_tiles[row_num] = 98 + opt_shift;
        pipe_tiles[row_num + 1] = 99 + opt_shift;
        pipe_tiles[row_num + 2] = 100 + opt_shift;
        pipe_tiles[row_num + 3] = 101 + opt_shift;
        pipe_tiles[row_num + 4] = 102 + opt_shift;
      } else if (j == 13) {
        pipe_tiles[row_num] = 106 + opt_shift;
        pipe_tiles[row_num + 1] = 107 + opt_shift;
        pipe_tiles[row_num + 2] = 108 + opt_shift;
        pipe_tiles[row_num + 3] = 109 + opt_shift;
        pipe_tiles[row_num + 4] = 110 + opt_shift;
      } else if (j == 14) {
        pipe_tiles[row_num] = 114 + opt_shift;
        pipe_tiles[row_num + 1] = 115 + opt_shift;
        pipe_tiles[row_num + 2] = 116 + opt_shift;
        pipe_tiles[row_num + 3] = 117 + opt_shift;
        pipe_tiles[row_num + 4] = 118 + opt_shift;
      } else if (j == 15) {
        pipe_tiles[row_num] = 122 + opt_shift;
        pipe_tiles[row_num + 1] = 123 + opt_shift;
        pipe_tiles[row_num + 2] = 124 + opt_shift;
        pipe_tiles[row_num + 3] = 125 + opt_shift;
        pipe_tiles[row_num + 4] = 126 + opt_shift;
      } else if (j == 16) {
        pipe_tiles[row_num] =
            pipe_tiles[row_num + 1] =
            pipe_tiles[row_num + 2] =
            pipe_tiles[row_num + 3] =
            pipe_tiles[row_num + 4] = 137;
      } else {
        pipe_tiles[row_num] =
            pipe_tiles[row_num + 1] =
            pipe_tiles[row_num + 2] =
            pipe_tiles[row_num + 3] =
            pipe_tiles[row_num + 4] = 0;
      }

    }
  }
  set_bkg_tiles(x, 0, PIPE_WIDTH, GRAPHICS_HEIGHT / SPRITE_HEIGHT, pipe_tiles);
}

void flush_row(UINT8 row) {
  UINT8 i = 0;
  UINT8 bkg_tiles[31];
  for (i = 0; i < 32; i++) {
    bkg_tiles[i] = 0;
  }
  set_bkg_tiles(0, row, 32, 1, bkg_tiles);
}

void flush_bkg() {
  UINT8 i = 0;
  for (i = 0; i < 32; i++) {
    flush_row(i);
  }
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
  UINT8 tiles[48];

  for (i = 0; i < 32; i+=2) {
    tiles[i] = 33;
    tiles[i+1] = 34;
  }
  set_bkg_tiles(0, 17, 32, 1, tiles);

  for (i = 0; i < 48; i++) {
    tiles[i] = i + 89;
  }

  for (i = 0; i < 4; i++) {
    set_bkg_tiles(i * 8 + 1, 11, 8, 5, tiles);
  }

  tiles[0] = 128;
  set_bkg_tiles(0, 15, 1, 1, tiles);

  for (i = 0; i < 32; i++) {
    tiles[i] = 137;
  };
  set_bkg_tiles(0, 16, 32, 1, tiles);
}

void draw_result(UINT16 result) {
  UINT8 text_tiles[6 * 7];
  UINT8 i;
  UINT8 best;
  UINT16 buffer;

  buffer = (UINT8) RAMPtr[0];
  if (buffer == 255) {
    best = 0;
  } else {
    best = buffer;
  }

  // hack for emulators that cant acceess to nv ram
  if (best == 0 && high_score_backup != 0) {
    best = high_score_backup;
  }

  if ((UINT8) result > best) {
    high_score_backup = result;
    RAMPtr[0] = result;
  }

  for (i = 0; i < 42; i++) {
    text_tiles[i] = 0;
  }

  fill_pipe_row_with_numbers(text_tiles, 14, result);
  fill_pipe_row_with_numbers(text_tiles, 38, best);

  text_tiles[0] = 66;
  text_tiles[1] = 67;
  text_tiles[2] = 68;
  text_tiles[3] = 86;
  text_tiles[4] = 87;
  text_tiles[5] = 69;

  text_tiles[24] = 88;
  text_tiles[25] = 67;
  text_tiles[26] = 68;
  text_tiles[27] = 69;

  set_bkg_tiles(7, 5, 6, 7, text_tiles);
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
  // const UINT8 player_height = flbird_tile_map_height;
  // // printf("%d %d\n", y, level);
  // if (y < level * SPRITE_HEIGHT + player_height * SPRITE_HEIGHT ||
  //     y > level * SPRITE_HEIGHT + PIPE_GAP * SPRITE_HEIGHT) {
  //   return TRUE;
  // } else {
    return FALSE;
  // }
}

INT16 get_player_y_pos(UINT8 t, UINT8 yd) {
  const UINT16 v0 = 200;
  const UINT8 g = 10;
  UINT16 dx0, dx1;

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
  static UINT16 bkg_position_x = 128;
  static UINT16 bkg_position_y = 160;
  static UINT16 counter = 0;
  UINT16 delta = 0;
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
  UINT8 steps[] = {
    71,
    72,
    73,
    74,
    75
  };
  UINT8 tile_num;

  delay(100);

  move_bkg(0, 0);

  if (step < sizeof(steps)) {
    tile_num = steps[step];
  } else {
    step = 0;
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

void dynamic_set_bkg_data(INT16 tile_count, UINT8 *tile_data) {
  // use 1 because 0 tile should be empty tile
  static INT16 last_free_tile_num = 1;

  set_bkg_data(last_free_tile_num, tile_count, tile_data);
  last_free_tile_num += tile_count;
}

void interrupt_LCD() {
  HIDE_WIN;
}

BOOLEAN cheat_code_inputed(UINT8 *code_buttons, UINT8 sizeof_code_buttons) {
  const UINT8 max_delay = 10;
  UINT8 joypad_value;
  static UINT8 counter = 0;
  static UINT8 code_current_button_position = 0;
  static BOOLEAN was_last_input_empty = FALSE;

  if (counter != max_delay) {
    counter++;
  }

  if (counter == max_delay) {
    code_current_button_position = 0;
  }

  joypad_value = joypad();

  if (joypad_value != 0) {
    counter = 0;
    if (was_last_input_empty == TRUE) {
      if (joypad_value & code_buttons[code_current_button_position]) {
        code_current_button_position++;
      } else {
        code_current_button_position = 0;
      }
    }
    if (code_current_button_position == sizeof_code_buttons) {
      counter = 0;
      code_current_button_position = 0;
      return TRUE;
    }
    was_last_input_empty = FALSE;
  } else {
    was_last_input_empty = TRUE;
  }
  return FALSE;
}

BOOLEAN reset_cheat_inputed() {
  const UINT8 code_buttons[] = {
    J_A,
    J_B,
    J_B,
    J_B
  };
  return cheat_code_inputed(&code_buttons, sizeof(code_buttons));
}

void stop_playing_intro() {
  NR12_REG = 0x00;
  NR22_REG = 0x00;
}

void play_intro() {
  static UINT8 counter = 0;

  if (counter == 127) {
    return;
  }

  if (counter == 0) {
    NR10_REG = 0x7C;
    NR11_REG = 0xB0;
    NR12_REG = 0x8F;
    NR13_REG = 0x10;
    NR14_REG = 0x84;

    NR21_REG = 0x60;
    NR22_REG = 0x8F;
    NR23_REG = 0x12;
    NR24_REG = 0x84;
  }

  if (clock() % 8) {
    counter++;
  }

  if (counter == 100) {
    stop_playing_intro();
    counter = 127;
  }
}

void play_fail() {
  NR41_REG = 0x01;
  NR42_REG = 0xA2;
  NR43_REG = 0x45;
  NR44_REG = 0xC0;
}

void play_jump() {
  NR10_REG = 0x36;
  NR11_REG = 0x10;
  NR12_REG = 0x83;
  NR13_REG = 0x00;
  NR14_REG = 0x87;
}

void play_c3() {
  NR21_REG = 0x82;
  NR22_REG = 0x81;
  NR23_REG = 0x10;
  NR24_REG = 0xC0;
}

void play_c4() {
  NR21_REG = 0x82;
  NR22_REG = 0x81;
  NR23_REG = 0xF0;
  NR24_REG = 0xC1;
}

void play_hh() {
  NR41_REG = 0x02;
  NR42_REG = 0x61;
  NR43_REG = 0x10;
  NR44_REG = 0xC0;
}

void play_snare() {
  NR41_REG = 0x02;
  NR42_REG = 0x61;
  NR43_REG = 0x42;
  NR44_REG = 0xC0;
}

void play_music() {
  static UINT8 step = 0;

  if (clock() % 8 != 0) {
    return;
  }

  play_hh();

  if (step == 1 || step == 6) {
    play_c4();
  } else {
    play_c3();
  }

  if (step == 3) {
      play_snare();
  }

  step++;

  if (step == 8) {
    step = 0;
  }
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
  UINT16 pipe_number = 1;
  UINT16 current_score = 0;
  UINT8 ttt[5];

  enum game_states current_game_state = TRANSITION_TO_TITLE;
  game_sprite_object player;
  game_sprite_object tutorial_arrow;
  game_sprite_object tutorial_a_button;

  NR52_REG = 0x80;
  NR51_REG = 0xFF;
  NR50_REG = 0x77;
  //
  // NR10_REG = 0x1E;
  // NR11_REG = 0x10;
  // NR12_REG = 0xF3;
  // NR13_REG = 0x00;
  // NR14_REG = 0x87;


  // // Mute channel 1 (there are other ways to do this)
  // NR12 = 0;
  // NR14 = 0x80;

  ENABLE_RAM_MBC1;

  RAMPtr = (UINT8 *)NON_VOLATILE_MEMORY_ADDRESS;

  new_gso(&player, flbird_tile_map_width, flbird_tile_map_height, &flbird_tile_data, &last_free_tile);
  new_gso(&tutorial_arrow, arrow_tile_map_width, arrow_tile_map_height, &arrow_tile_data, &last_free_tile);
  new_gso(&tutorial_a_button, ab_tile_map_width, ab_tile_map_height, &ab_tile_data, &last_free_tile);

  dynamic_set_bkg_data(flbody_tile_count, flbody_tile_data);

  // set_bkg_data(1, flbody_tile_count, flbody_tile_data);
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

  set_bkg_data(76, num_0_tile_count, num_0_tile_data);
  set_bkg_data(77, num_1_tile_count, num_1_tile_data);
  set_bkg_data(78, num_2_tile_count, num_2_tile_data);
  set_bkg_data(79, num_3_tile_count, num_3_tile_data);
  set_bkg_data(80, num_4_tile_count, num_4_tile_data);
  set_bkg_data(81, num_5_tile_count, num_5_tile_data);
  set_bkg_data(82, num_6_tile_count, num_6_tile_data);
  set_bkg_data(83, num_7_tile_count, num_7_tile_data);
  set_bkg_data(84, num_8_tile_count, num_8_tile_data);
  set_bkg_data(85, num_9_tile_count, num_9_tile_data);

  set_bkg_data(86, u_tile_count, u_tile_data);
  set_bkg_data(87, l_tile_count, l_tile_data);
  set_bkg_data(88, b_tile_count, b_tile_data);

  set_bkg_data(89, bg_tile_count, bg_tile_data);
  set_bkg_data(137, grass_tile_count, grass_tile_data);

  flush_bkg();

  // Interupts causes freezes, bad idea

  // STAT_REG = 0x45;
  // LYC_REG = 0x08;            //  Fire LCD Interupt on the 8th scan line

  disable_interrupts();

  SPRITES_8x8;
  SHOW_BKG;
  // SHOW_WIN;
  SHOW_SPRITES;
  DISPLAY_ON;

  // add_LCD(interrupt_LCD);
  // enable_interrupts();

  // set_interrupts(VBL_IFLAG | LCD_IFLAG);

  for (i = 0; i < 5; i++) {
    ttt[i] = 1;
  }

  // fill_pipe_row_with_numbers(ttt, 0, (INT16) RAMPtr[0]);

  // set_bkg_tiles(2, 2, 5, 1, ttt);

  draw_land();

  draw_title();
  move_bkg(0, 160);

  // move_win(0, 0);
  // for (i = 0; i < 31; i++) {
  //   test[i] = 0;
  // }
  // for (i = 0; i < 18; i++) {
  //   set_win_tiles(0, i, 20, 1, test);
  // }
  // i = 0;

  first_pipe_collision_zone_start_x = 10 * 8 - (player_position_x + (3 * 4));
  first_pipe_collision_zone_end_x = first_pipe_collision_zone_start_x + 40 + 2 * 8;
  second_pipe_collision_zone_start_x = 255 - 40 - player_position_x - 3 * 4;
  second_pipe_collision_zone_end_x = second_pipe_collision_zone_start_x + 40 + 2 * 8;

  move_gso(&tutorial_arrow, tutorial_arrow_position_x, tutorial_arrow_position_y);
  move_gso(&tutorial_a_button, 168, 80);

  player_position_x = 0;
  player_position_y = 144-50;

  play_intro();

  while(resume) {
    // Main loop

    // TODO: one pressing – one jump
    j = joypad();

    // SHOW_WIN;

    if (current_game_state == TRANSITION_TO_TITLE) {
      if (move_title_in() == TRUE) {
        current_game_state = TITLE;
      };

      play_intro();
    }

    if (current_game_state == TITLE) {
      if (j & J_START) {
        initrand(DIV_REG);
        current_game_state = TRANSITION_TO_TUTORIAL;
      }

      if (reset_cheat_inputed() == TRUE) {
        RAMPtr[0] = 0;
        resume = FALSE;
        reset();
      }

      play_intro();
    }

    if (current_game_state == TRANSITION_TO_TUTORIAL) {
      stop_playing_intro();

      if (move_title_out() == TRUE) {
        current_game_state = TUTORIAL;
      };
    }

    if (current_game_state == TUTORIAL) {
      play_music();
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
      } else {
        player_position_x = 50;
        move_gso(&player, player_position_x, player_position_y);
      }

      if (j & J_A && !jump_is_delayed && player_position_x == 50) {
        time_backup = clock();
        delaying = clock() + JUMP_DELAY;
        jump_is_delayed = TRUE;
        scroll_position_x = 0;
        pipe_number = 1;
        current_score = 0;
        play_jump();
        current_game_state = MAIN;
      }
    }

    if (current_game_state == MAIN) {
      play_music();
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
        draw_pipe(27, second_pipe_current_level, pipe_number++);
      }
      if (scroll_position_x > SECOND_PIPE_POSITION_X && scroll_position_x <= SECOND_PIPE_POSITION_X + GAME_BKG_SCROLL_STEP) {
        first_pipe_current_level = get_random_pipe_level();
        draw_pipe(10, first_pipe_current_level, pipe_number++);
        pipe_collision_check_is_needed = TRUE;
      }

      if (j & J_A && !jump_is_delayed) {
        yd = GRAPHICS_HEIGHT - player_position_y;
        time_backup = clock();
        delaying = clock() + JUMP_DELAY;
        jump_is_delayed = TRUE;
        play_jump();
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

      if (((scroll_position_x > first_pipe_collision_zone_end_x &&
          scroll_position_x <= first_pipe_collision_zone_end_x + GAME_BKG_SCROLL_STEP) ||
          (scroll_position_x > second_pipe_collision_zone_end_x &&
          scroll_position_x <= second_pipe_collision_zone_end_x + GAME_BKG_SCROLL_STEP)) &&
          pipe_collision_check_is_needed) {
        current_score++;
      }

      if (was_collision_flag == TRUE) {
        play_fail();
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

    if (current_game_state == RETRY) {
      draw_result(current_score);
      delay(2000);
      current_game_state = TUTORIAL;
      move_gso(&tutorial_arrow, tutorial_arrow_position_x, tutorial_arrow_position_y);
      move_gso(&tutorial_a_button, 168, 80);

      player_position_x = 0;
      player_position_y = 144-50;
      yd = 50;
      time_backup = 0;
      delaying = 0;
      jump_is_delayed = FALSE;
      pipe_collision_check_is_needed = FALSE;

      flush_bkg();
      draw_land();
      draw_title();
      move_bkg(0, 160);
    }

    if (current_game_state == TRANSITION_TO_RETRY) {
      move_gso(&player, 200, 200);
      if (blackify_back() == TRUE) {
        flush_bkg();
        current_game_state = RETRY;
      }
    }

    wait_vbl_done();
  }
}
