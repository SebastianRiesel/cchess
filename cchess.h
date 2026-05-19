#pragma once

#include <stdbool.h>
#include <stddef.h>
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
  bool pawn_is_en_passantable;
} cchess_piece_t;

// chess board from whites perspective, a1 == x = 0, y = 0
typedef struct s_cchess_board {
  cchess_piece_t grid[64];
} cchess_board_t;

typedef enum {
  CCHESS_MOVE_NORMAL,
  CCHESS_MOVE_PAWN_DOUBLE,
  CCHESS_MOVE_CASTLE,
  CCHESS_MOVE_EN_PASSANT,
  CCHESS_MOVE_PROMOTION
} cchess_move_type_t;

typedef struct {
  size_t x1;
  size_t y1;
  size_t x2;
  size_t y2;
  cchess_move_type_t type;
  cchess_piece_type_t promotion_type;

} cchess_move_t;

void cchess_board_clear(cchess_board_t *board);
void cchess_board_init(cchess_board_t *board);
cchess_piece_t cchess_board_get_piece(cchess_board_t *board, size_t x,
                                      size_t y);
cchess_piece_t cchess_board_set_piece(cchess_board_t *board, size_t x, size_t y,
                                      cchess_piece_t piece);
cchess_piece_t cchess_board_raw_move(cchess_board_t *board, size_t x1,
                                     size_t y1, size_t x2, size_t y2);
void cchess_board_move(cchess_board_t *board, cchess_move_t move);

char cchess_piece_type_to_char(cchess_piece_type_t pt);
int cchess_move_to_string(cchess_move_t move, char *dest, size_t size);
int cchess_piece_to_string(cchess_piece_t piece, char *dest, size_t size);
int cchess_board_to_string(cchess_board_t *board, char *dest, size_t size);

bool cchess_board_color_is_in_check(cchess_board_t *board,
                                    cchess_color_t color);

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
} cchess_game_t;

size_t cchess_board_unfiltered_moves(
    cchess_board_t *board, cchess_color_t color,
    cchess_move_t **moves); // calculates all possible moves, without checking
                            // for checks and King safety
size_t cchess_board_moves(
    cchess_board_t *board, cchess_color_t color,
    cchess_move_t **moves); // calculates all possible moves, without the moves
                            // that violate king safety

typedef struct cchess_player_s cchess_player_t;

typedef cchess_move_t (*cchess_player_input_callback)(cchess_player_t *self,
                                                      cchess_move_t *moves,
                                                      size_t len);
typedef void (*cchess_player_output_callback)(cchess_player_t *self,
                                              cchess_game_t game);

struct cchess_player_s {
  cchess_player_output_callback out;
  cchess_player_input_callback in;
};

cchess_game_t cchess_play_game(cchess_player_t *white, cchess_player_t *black);
