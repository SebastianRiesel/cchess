/* cchess - A small hobby chess lirbary and engine
 * Copyright (C) 2026 Sebastian Riesel
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Contact:
 * sebastian.riesel06@gmail.com
 */

#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

typedef enum { CCHESS_COLOR_WHITE = 0, CCHESS_COLOR_BLACK = 1 } cchess_color_t;

#define CCHESS_COLOR_OTHER_COLOR(c)                                            \
  ((c) == CCHESS_COLOR_WHITE ? CCHESS_COLOR_BLACK : CCHESS_COLOR_WHITE)

typedef enum {
  CCHESS_PIECE_NONE = 0,
  CCHESS_PIECE_PAWN = 1,
  CCHESS_PIECE_ROOK = 2,
  CCHESS_PIECE_BISHOP = 3,
  CCHESS_PIECE_KNIGHT = 4,
  CCHESS_PIECE_QUEEN = 5,
  CCHESS_PIECE_KING = 6
} cchess_piece_type_t;

typedef struct {
  cchess_color_t color;
  cchess_piece_type_t type;
  bool has_moved;
} cchess_piece_t;

// chess board from whites perspective, a1 == x = 0, y = 0
typedef struct s_cchess_board {
  cchess_piece_t grid[64];
} cchess_board_t;


#define ARR_INDEX(x, y) ((y) * 8 + (x))


typedef enum {
  CCHESS_MOVE_NORMAL,
  CCHESS_MOVE_PAWN_DOUBLE,
  CCHESS_MOVE_CASTLE,
  CCHESS_MOVE_EN_PASSANT,
  CCHESS_MOVE_PROMOTION
} cchess_move_type_t;

typedef struct {
  int8_t x1;
  int8_t y1;
  int8_t x2;
  int8_t y2;
  cchess_move_type_t type;
  cchess_piece_type_t promotion_type;
} cchess_move_t;

typedef struct {
  cchess_move_t move;
  cchess_piece_t captured_piece; // for reversing the move
  cchess_piece_t moved_piece;    // original piece
  int8_t prev_en_passant_x;
  int8_t prev_en_passant_y;

} cchess_stored_move_t;

typedef struct {
  cchess_stored_move_t *moves;
  size_t size;
  size_t capacity;
} cchess_stored_move_stack_t;

cchess_stored_move_stack_t *cchess_stored_move_stack_create();
void cchess_stored_move_stack_destroy(cchess_stored_move_stack_t *stack);
void cchess_stored_move_stack_push(cchess_stored_move_stack_t *stack,
                                   cchess_stored_move_t move);
cchess_stored_move_t
cchess_stored_move_stack_pop(cchess_stored_move_stack_t *stack);

size_t cchess_stored_move_stack_size(cchess_stored_move_stack_t *stack);

typedef enum {
  CCHESS_STATE_RUNNING,
  CCHESS_STATE_WHITE_WON,
  CCHESS_STATE_BLACK_WON,
  CCHESS_STATE_STALEMATE
} cchess_game_state_t;

typedef struct {
  cchess_board_t board;
  cchess_color_t current_color;
  cchess_game_state_t state;

  int en_passant_x;
  int en_passant_y;

  cchess_stored_move_stack_t *moves;
} cchess_game_t;

typedef struct cchess_player_s cchess_player_t;

typedef cchess_move_t (*cchess_player_input_callback)(cchess_player_t *self,
                                                      cchess_move_t *moves,
                                                      size_t len);
typedef void (*cchess_player_output_callback)(cchess_player_t *self,
                                              cchess_game_t* game);

struct cchess_player_s {
  cchess_player_output_callback out;
  cchess_player_input_callback in;
};

void cchess_board_clear(cchess_board_t *board);
void cchess_board_init(cchess_board_t *board);
cchess_piece_t cchess_board_get_piece(cchess_board_t *board, int8_t x, int8_t y);
cchess_piece_t cchess_board_set_piece(cchess_board_t *board, int8_t x, int8_t y,
                                      cchess_piece_t piece);
cchess_piece_t cchess_board_raw_move(cchess_board_t *board, int8_t x1,
                                     int8_t y1, int8_t x2, int8_t y2);
void cchess_game_move(cchess_game_t *game, cchess_move_t move);
void cchess_game_reverse_move(cchess_game_t *game);

char cchess_piece_type_to_char(cchess_piece_type_t pt);
int cchess_move_to_string(cchess_move_t move, char *dest, size_t size);
int cchess_piece_to_string(cchess_piece_t piece, char *dest, size_t size);
int cchess_board_to_string(cchess_board_t *board, char *dest, size_t size);

bool cchess_game_is_checking(cchess_game_t *game);
bool cchess_game_is_in_check(cchess_game_t *game);

size_t cchess_game_unfiltered_moves(
    cchess_game_t *game,
    cchess_move_t *move_buf); // calculates all possible moves, without checking
// for checks and King safety
size_t cchess_game_moves(
    cchess_game_t *game,
    cchess_move_t *move_buf); // calculates all possible moves, without the moves
// that violate king safety

cchess_game_t cchess_play_game(cchess_player_t *white, cchess_player_t *black);
