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

#include "cchess.h"
#include <SDL3/SDL_log.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MOVE_BUF_SIZE 250

#define STORED_MOVE_STACK_INIT_CAP 100
cchess_stored_move_stack_t *cchess_stored_move_stack_create() {
  cchess_stored_move_t *moves = (cchess_stored_move_t *)malloc(
      sizeof(cchess_stored_move_t) * STORED_MOVE_STACK_INIT_CAP);

  cchess_stored_move_stack_t *stack =
      (cchess_stored_move_stack_t *)malloc(sizeof(cchess_stored_move_stack_t));
  *stack = (cchess_stored_move_stack_t){moves, 0, STORED_MOVE_STACK_INIT_CAP};

  return stack;
}

void cchess_stored_move_stack_destroy(cchess_stored_move_stack_t *stack) {
  free(stack->moves);
  free(stack);
}

void cchess_stored_move_stack_push(cchess_stored_move_stack_t *stack,
                                   cchess_stored_move_t move) {
  if (stack->size >= stack->capacity) {
    stack->moves = realloc(stack->moves,
                           stack->capacity * 2 * sizeof(cchess_stored_move_t));
    stack->capacity *= 2;
  }

  stack->moves[stack->size] = move;
  stack->size++;
}

cchess_stored_move_t
cchess_stored_move_stack_pop(cchess_stored_move_stack_t *stack) {
  cchess_stored_move_t move = stack->moves[--(stack->size)];
  return move;
}

size_t cchess_stored_move_stack_size(cchess_stored_move_stack_t *stack) {
  return stack->size;
}

void cchess_board_clear(cchess_board_t *board) {
  for (int8_t i = 0; i < 64; i++) {
    cchess_piece_t p = {CCHESS_COLOR_WHITE, CCHESS_PIECE_NONE, 0};
    board->grid[i] = p;
  }
}

void cchess_board_init(cchess_board_t *board) {
  cchess_board_clear(board);

#define BGA(x, y) (board->grid[ARR_INDEX(x, y)])
#define CCP(color, type) ((cchess_piece_t){color, type, 0})
  // Rooks
  BGA(0, 0) = CCP(CCHESS_COLOR_WHITE, CCHESS_PIECE_ROOK);
  BGA(7, 0) = CCP(CCHESS_COLOR_WHITE, CCHESS_PIECE_ROOK);
  BGA(0, 7) = CCP(CCHESS_COLOR_BLACK, CCHESS_PIECE_ROOK);
  BGA(7, 7) = CCP(CCHESS_COLOR_BLACK, CCHESS_PIECE_ROOK);

  // Knights
  BGA(1, 0) = CCP(CCHESS_COLOR_WHITE, CCHESS_PIECE_KNIGHT);
  BGA(6, 0) = CCP(CCHESS_COLOR_WHITE, CCHESS_PIECE_KNIGHT);
  BGA(1, 7) = CCP(CCHESS_COLOR_BLACK, CCHESS_PIECE_KNIGHT);
  BGA(6, 7) = CCP(CCHESS_COLOR_BLACK, CCHESS_PIECE_KNIGHT);

  // Bishops
  BGA(2, 0) = CCP(CCHESS_COLOR_WHITE, CCHESS_PIECE_BISHOP);
  BGA(5, 0) = CCP(CCHESS_COLOR_WHITE, CCHESS_PIECE_BISHOP);
  BGA(2, 7) = CCP(CCHESS_COLOR_BLACK, CCHESS_PIECE_BISHOP);
  BGA(5, 7) = CCP(CCHESS_COLOR_BLACK, CCHESS_PIECE_BISHOP);

  // Queens and Kings
  BGA(3, 0) = CCP(CCHESS_COLOR_WHITE, CCHESS_PIECE_QUEEN);
  BGA(4, 0) = CCP(CCHESS_COLOR_WHITE, CCHESS_PIECE_KING);
  BGA(3, 7) = CCP(CCHESS_COLOR_BLACK, CCHESS_PIECE_QUEEN);
  BGA(4, 7) = CCP(CCHESS_COLOR_BLACK, CCHESS_PIECE_KING);

  // Pawns
  for (int8_t x = 0; x < 8; x++) {
    BGA(x, 1) = CCP(CCHESS_COLOR_WHITE, CCHESS_PIECE_PAWN);
    BGA(x, 6) = CCP(CCHESS_COLOR_BLACK, CCHESS_PIECE_PAWN);
  }

#undef CCP
#undef BGA
}


cchess_piece_t cchess_board_get_piece(cchess_board_t *board, int8_t x, int8_t y) {
  return board->grid[ARR_INDEX((x), (y))];
}


cchess_piece_t cchess_board_set_piece(cchess_board_t *board, int8_t x, int8_t y,
                                      cchess_piece_t piece) {
  cchess_piece_t p = board->grid[ARR_INDEX(x, y)];
  board->grid[ARR_INDEX(x, y)] = piece;
  return p;
}

cchess_piece_t cchess_board_raw_move(cchess_board_t *board, int8_t x1,
                                     int8_t y1, int8_t x2, int8_t y2) {
  cchess_piece_t p = board->grid[ARR_INDEX(x2, y2)];
  board->grid[ARR_INDEX(x2, y2)] = board->grid[ARR_INDEX(x1, y1)];
  board->grid[ARR_INDEX(x1, y1)] =
      (cchess_piece_t){CCHESS_COLOR_WHITE, CCHESS_PIECE_NONE, 0};
  return p;
}

int cchess_move_to_string(cchess_move_t move, char *dest, size_t size) {
  int res;
  res =
      snprintf(dest, size, "cchess_normal_move_t[x1=%d,y1=%d,x2=%d,y2=%d]",
               move.x1, move.y1, move.x2, move.y2);

  if (res < 0) {
    return 0;
  }
  if ((size_t)res >= size) {
    return size;
  }
  return res;
}

char cchess_piece_type_to_char(cchess_piece_type_t pt) {
  char p;

  switch (pt) {
  case CCHESS_PIECE_NONE:
    p = '.';
    break;
  case CCHESS_PIECE_PAWN:
    p = 'p';
    break;
  case CCHESS_PIECE_KNIGHT:
    p = 'h';
    break;
  case CCHESS_PIECE_BISHOP:
    p = 'b';
    break;
  case CCHESS_PIECE_ROOK:
    p = 'r';
    break;
  case CCHESS_PIECE_QUEEN:
    p = 'q';
    break;
  case CCHESS_PIECE_KING:
    p = 'k';
    break;
  }
  return p;
}

int cchess_piece_to_string(cchess_piece_t piece, char *dest, size_t size) {
  char p = cchess_piece_type_to_char(piece.type);

  if (piece.type != CCHESS_PIECE_NONE && piece.color == CCHESS_COLOR_BLACK) {
    p = p - 32;
  }

  if (size >= 2 && dest != NULL) {
    dest[0] = p;
    dest[1] = '\0';
    return 1;
  }
  return 0;
}

const char *row_str = "  12345678  \n";

int cchess_board_to_string(cchess_board_t *board, char *dest, size_t size) {

  size_t row_len = strlen(row_str);
  if (size < row_len) {
    return 0;
  }

  size_t n = row_len;
  strncpy(dest, row_str, row_len + 1);
  for (int8_t y = 7; y >= 0; y--) {
    if (size - n <= 2) {
      return n;
    }
    dest[n++] = ((char)y) + 49;
    dest[n++] = ' ';
    dest[n] = '\0';

    for (int8_t x = 0; x < 8; x++) {
      if (size - n <= 1) {
        return n;
      }

      size_t result = cchess_piece_to_string(board->grid[ARR_INDEX(x, y)],
                                             dest + n, size - n);
      n += result;
    }
    if (size - n <= 3) {
      return n;
    }
    dest[n++] = ' ';
    dest[n++] = ((char)y) + 49;
    dest[n++] = '\n';
    dest[n] = '\0';
  }
  if (size - n <= row_len) {
    return 0;
  }

  strncpy(dest + n, row_str, row_len + 1);
  n += row_len;
  return n;
}

// lookup table for knight positions
const int8_t knight_x_offsets[] = {-1, 1, 2, 2, 1, -1, -2, -2};
const int8_t knight_y_offsets[] = {2, 2, 1, -1, -2, -2, 1, -1};

// diagonal walking directions
const int8_t diagional_x_directions[] = {1, 1, -1, -1};
const int8_t diagional_y_directions[] = {1, -1, 1, -1};

// straight walking directions
const int8_t straight_x_directions[] = {1, -1, 0, 0};
const int8_t straight_y_directions[] = {0, 0, 1, -1};

#define APPEND_MOVE(move_buf, len, move)                                       \
  {                                                                            \
    move_buf[len] = move;                                                      \
    len++;                                                                     \
  }
#define M(x1, y1, x2, y2)                                                      \
  ((cchess_move_t){x1, y1, x2, y2, CCHESS_MOVE_NORMAL, CCHESS_PIECE_NONE})

size_t append_walk(cchess_board_t *board, cchess_color_t color, int8_t x,
                   int8_t y, int8_t x_direction, int8_t y_direction,
                   cchess_move_t *moves) {
  size_t buf_len = 0;
  int8_t x2 = x;
  int8_t y2 = y;
  while (1) {
    x2 += x_direction;
    y2 += y_direction;

    if (x2 < 0 || x2 > 7 || y2 < 0 || y2 > 7)
      break;

    cchess_piece_t other_piece = cchess_board_get_piece(board, x2, y2);
    if (other_piece.type == CCHESS_PIECE_NONE) {
      APPEND_MOVE(moves, buf_len, M(x, y, x2, y2));
    } else if (other_piece.color == color) {
      break; // run into piece of same color
    } else {
      APPEND_MOVE(moves, buf_len,
                  M(x, y, x2, y2)); // capture piece of different color
      break;
    }
  }

  return buf_len;
}

size_t cchess_game_unfiltered_moves(cchess_game_t *game,
                                    cchess_move_t *move_buf) {
  cchess_board_t *board = &game->board;
  cchess_color_t color = game->current_color;
  // assume there are no positions, where more than 500 moves are possible
  //cchess_move_t *move_buf = (cchess_move_t *)calloc(500, sizeof(cchess_move_t));
  size_t buf_len = 0;
  for (int8_t y = 0; y < 8; y++) {
    for (int8_t x = 0; x < 8; x++) {
      // size_t i = ARR_INDEX(x,y);
      cchess_piece_t piece = cchess_board_get_piece(board, x, y);
      if (piece.color != color) {
        continue;
      }
      int8_t pawn_direction = color == CCHESS_COLOR_WHITE ? 1 : -1;
      int8_t pawn_start_rank = color == CCHESS_COLOR_WHITE ? 1 : 6;
      int8_t pawn_promotion_rank = color == CCHESS_COLOR_WHITE ? 7 : 0;
      switch (piece.type) {
      case CCHESS_PIECE_NONE:
        break;
      case CCHESS_PIECE_PAWN:
        if ((color == CCHESS_COLOR_WHITE && y < 7) ||
            (color == CCHESS_COLOR_BLACK && y > 0)) {
          if (cchess_board_get_piece(board, x, y + pawn_direction).type ==
              CCHESS_PIECE_NONE) {
            cchess_move_t move = M(x, y, x, y + pawn_direction);
            if (y + pawn_direction == pawn_promotion_rank) {
              move.type = CCHESS_MOVE_PROMOTION;
              move.promotion_type = CCHESS_PIECE_QUEEN;
            }
            APPEND_MOVE(move_buf, buf_len, move);
            if (y == pawn_start_rank &&
                cchess_board_get_piece(board, x, y + 2 * pawn_direction).type ==
                    CCHESS_PIECE_NONE) {
              cchess_move_t move = M(x, y, x, y + 2 * pawn_direction);
              move.type = CCHESS_MOVE_PAWN_DOUBLE;
              APPEND_MOVE(move_buf, buf_len, move);
            }
          }
          if (x < 7) {
            cchess_piece_t diag_piece =
                cchess_board_get_piece(board, x + 1, y + pawn_direction);
            if (diag_piece.type != CCHESS_PIECE_NONE &&
                diag_piece.color != color) {
              cchess_move_t move = M(x, y, x + 1, y + pawn_direction);
              if (y + pawn_direction == pawn_promotion_rank) {
                move.type = CCHESS_MOVE_PROMOTION;
                move.promotion_type = CCHESS_PIECE_QUEEN;
              }
              APPEND_MOVE(move_buf, buf_len, move);
            }

            cchess_piece_t neighbour_piece =
                cchess_board_get_piece(board, x + 1, y);
            if (neighbour_piece.type == CCHESS_PIECE_PAWN &&
                neighbour_piece.color != piece.color &&
                game->en_passant_x == (x + 1) &&
                game->en_passant_y == y) {
              cchess_move_t move = M(x, y, x + 1, y + pawn_direction);
              move.type = CCHESS_MOVE_EN_PASSANT;
              APPEND_MOVE(move_buf, buf_len, move);
            }
          }
          if (x > 0) {
            cchess_piece_t diag_piece =
                cchess_board_get_piece(board, x - 1, y + pawn_direction);
            if (diag_piece.type != CCHESS_PIECE_NONE &&
                diag_piece.color != color) {
              cchess_move_t move = M(x, y, x - 1, y + pawn_direction);
              if (y + pawn_direction == pawn_promotion_rank) {
                move.type = CCHESS_MOVE_PROMOTION;
                move.promotion_type = CCHESS_PIECE_QUEEN;
              }
              APPEND_MOVE(move_buf, buf_len, move);
            }
            cchess_piece_t neighbour_piece =
                cchess_board_get_piece(board, x - 1, y);
            if (neighbour_piece.type == CCHESS_PIECE_PAWN &&
                neighbour_piece.color != piece.color &&
                game->en_passant_x == (x - 1) &&
                game->en_passant_y == y) {
              cchess_move_t move = M(x, y, x - 1, y + pawn_direction);
              move.type = CCHESS_MOVE_EN_PASSANT;
              APPEND_MOVE(move_buf, buf_len, move);
            }
          }
        }
        break;
      case CCHESS_PIECE_KNIGHT:
        for (size_t k = 0; k < 8; k++) {

          int8_t x2 = x + knight_x_offsets[k];
          int8_t  y2 = y + knight_y_offsets[k];

          if (x2 < 0 || x2 > 7 || y2 < 0 || y2 > 7) {
            continue;
          }

          cchess_piece_t other = cchess_board_get_piece(board, x2, y2);
          if (other.type == CCHESS_PIECE_NONE || other.color != color) {
            APPEND_MOVE(move_buf, buf_len, M(x, y, x2, y2));
          }
        }
        break;
      case CCHESS_PIECE_BISHOP:
        for (size_t k = 0; k < 4; k++) {
          int8_t x_dir = diagional_x_directions[k];
          int8_t y_dir = diagional_y_directions[k];
          size_t n = append_walk(board, color, x, y, x_dir, y_dir,
                                 (move_buf + buf_len));
          buf_len += n;
        }
        break;
      case CCHESS_PIECE_ROOK:
        for (size_t k = 0; k < 4; k++) {
          int8_t x_dir = straight_x_directions[k];
          int8_t y_dir = straight_y_directions[k];
          size_t n = append_walk(board, color, x, y, x_dir, y_dir,
                                 (move_buf + buf_len));
          buf_len += n;
        }
        break;
      case CCHESS_PIECE_QUEEN:
        for (size_t k = 0; k < 4; k++) {
          int8_t x_dir = straight_x_directions[k];
          int8_t y_dir = straight_y_directions[k];
          size_t n = append_walk(board, color, x, y, x_dir, y_dir,
                                 (move_buf + buf_len));
          buf_len += n;
        }
        for (size_t k = 0; k < 4; k++) {
          int8_t x_dir = diagional_x_directions[k];
          int8_t y_dir = diagional_y_directions[k];
          size_t n = append_walk(board, color, x, y, x_dir, y_dir,
                                 (move_buf + buf_len));
          buf_len += n;
        }
        break;
      case CCHESS_PIECE_KING:
        for (size_t k = 0; k < 4; k++) {
          int8_t x_dir = straight_x_directions[k];
          int8_t y_dir = straight_y_directions[k];
          int8_t x2 = x + x_dir;
          int8_t y2 = y + y_dir;
          if (x2 < 0 || x2 > 7 || y2 < 0 || y2 > 7) {
            continue;
          }
          cchess_piece_t other_piece = cchess_board_get_piece(board, x2, y2);
          if (other_piece.type == CCHESS_PIECE_NONE) {
            APPEND_MOVE(move_buf, buf_len, M(x, y, x2, y2));
          } else if (other_piece.color != color) {
            APPEND_MOVE(move_buf, buf_len,
                        M(x, y, x2, y2)); // capture piece of different color
          }
        }
        for (size_t k = 0; k < 4; k++) {
          int8_t x_dir = diagional_x_directions[k];
          int8_t y_dir = diagional_y_directions[k];
          int8_t x2 = x + x_dir;
          int8_t y2 = y + y_dir;
          if (x2 < 0 || x2 > 7 || y2 < 0 || y2 > 7) {
            continue;
          }
          cchess_piece_t other_piece = cchess_board_get_piece(board, x2, y2);
          if (other_piece.type == CCHESS_PIECE_NONE) {
            APPEND_MOVE(move_buf, buf_len, M(x, y, x2, y2));
          } else if (other_piece.color != color) {
            APPEND_MOVE(move_buf, buf_len,
                        M(x, y, x2, y2)); // capture piece of different color
          }
        }

        if (!piece.has_moved) {
          cchess_piece_t left_piece = cchess_board_get_piece(board, 0, y);
          if ((!left_piece.has_moved) && left_piece.type == CCHESS_PIECE_ROOK) {
            bool castle_possible = true;
            for (int8_t x2 = x - 1; x2 > 0; x2--) {
              if (cchess_board_get_piece(board, x2, y).type !=
                  CCHESS_PIECE_NONE) {
                castle_possible = false;
              }
            }
            if (castle_possible) {
              cchess_move_t move = M(x, y, x - 2, y);
              move.type = CCHESS_MOVE_CASTLE;
              APPEND_MOVE(move_buf, buf_len, move);
            }
          }
          cchess_piece_t right_piece = cchess_board_get_piece(board, 7, y);
          if ((!right_piece.has_moved) &&
              right_piece.type == CCHESS_PIECE_ROOK) {
            bool castle_possible = true;
            for (int8_t x2 = x + 1; x2 < 7; x2++) {
              if (cchess_board_get_piece(board, x2, y).type !=
                  CCHESS_PIECE_NONE) {
                castle_possible = false;
              }
            }
            if (castle_possible) {
              cchess_move_t move = M(x, y, x + 2, y);
              move.type = CCHESS_MOVE_CASTLE;
              APPEND_MOVE(move_buf, buf_len, move);
            }
          }
        }

        break;
      }
    }
  }
  return buf_len;
}
#undef APPEND_MOVE

void cchess_game_move(cchess_game_t *game, cchess_move_t move) {
  cchess_board_t *board = &game->board;
  cchess_stored_move_t stored_move;
  stored_move.move = move;

  stored_move.prev_en_passant_x = game->en_passant_x;
  stored_move.prev_en_passant_y = game->en_passant_y;

  cchess_piece_t piece = cchess_board_get_piece(board, move.x1, move.y1);

  stored_move.moved_piece = piece;

  // if pawn makes double move, it is en_passantable next turn
  if (move.type == CCHESS_MOVE_PAWN_DOUBLE) {
    game->en_passant_x = move.x1;
    game->en_passant_y = move.y2;
  } else {
    game->en_passant_x = -2;
    game->en_passant_y = -2;
  }

  piece.has_moved = true;

  // commit piece values onto board
  cchess_board_set_piece(board, move.x1, move.y1, piece);

  stored_move.captured_piece = cchess_board_get_piece(board, move.x2, move.y2);
  // make move
  cchess_board_raw_move(board, move.x1, move.y1, move.x2, move.y2);

  if (move.type == CCHESS_MOVE_PROMOTION) {

    cchess_board_set_piece(
        board, move.x2, move.y2,
        (cchess_piece_t){piece.color, move.promotion_type, true});
  }

  if (move.type == CCHESS_MOVE_CASTLE) {
    if ((move.x2) > (move.x1)) {
      cchess_board_raw_move(board, 7, move.y1, move.x2 - 1, move.y1);
    } else if ((move.x2) < (move.x1)) {
      cchess_board_raw_move(board, 0, move.y1, move.x2 + 1, move.y1);
    } else {
      printf("Error! Castle move data malformed!");
    }
  }

  if (move.type == CCHESS_MOVE_EN_PASSANT) {
    if ((move.y2) > (move.y1)) {
      stored_move.captured_piece =
          cchess_board_get_piece(board, move.x2, move.y2 - 1);
      cchess_board_set_piece(board, move.x2, move.y2 - 1,
                             (cchess_piece_t){CCHESS_COLOR_WHITE,
                                              CCHESS_PIECE_NONE, false});
    } else if ((move.y2) < (move.y1)) {
      stored_move.captured_piece =
          cchess_board_get_piece(board, move.x2, move.y2 + 1);
      cchess_board_set_piece(board, move.x2, move.y2 + 1,
                             (cchess_piece_t){CCHESS_COLOR_WHITE,
                                              CCHESS_PIECE_NONE, false});
    } else {
      printf("Error! en-passant move data malformed!");
    }
  }

  game->current_color = CCHESS_COLOR_OTHER_COLOR(game->current_color);
  cchess_stored_move_stack_push(game->moves, stored_move);
}

void cchess_game_reverse_move(cchess_game_t *game) {
  cchess_stored_move_t stored_move = cchess_stored_move_stack_pop(game->moves);
  cchess_move_t move = stored_move.move;
  game->en_passant_x = stored_move.prev_en_passant_x;
  game->en_passant_y = stored_move.prev_en_passant_y;

  game->current_color = CCHESS_COLOR_OTHER_COLOR(game->current_color);

  switch (move.type) {
  case CCHESS_MOVE_PAWN_DOUBLE:
  case CCHESS_MOVE_PROMOTION:
  case CCHESS_MOVE_NORMAL:
    cchess_board_set_piece(&game->board, move.x1, move.y1,
                           stored_move.moved_piece);
    cchess_board_set_piece(&game->board, move.x2, move.y2,
                           stored_move.captured_piece);

    break;
  case CCHESS_MOVE_CASTLE:
    cchess_board_set_piece(&game->board, move.x1, move.y1,
                           stored_move.moved_piece);
    cchess_board_set_piece(
        &game->board, move.x2, move.y2,
        (cchess_piece_t){CCHESS_COLOR_WHITE, CCHESS_PIECE_NONE, false});
    if ((move.x2) > (move.x1)) {
      cchess_piece_t rook =
          cchess_board_get_piece(&game->board, move.x2 - 1, move.y1);
      rook.has_moved = false;
      cchess_board_set_piece(&game->board, 7, move.y1, rook);
      cchess_board_set_piece(&game->board, move.x2 - 1, move.y1, (cchess_piece_t) {CCHESS_COLOR_WHITE, CCHESS_PIECE_NONE, false});
    } else if ((move.x2) < (move.x1)) {
      cchess_piece_t rook =
          cchess_board_get_piece(&game->board, move.x2 + 1, move.y1);
      rook.has_moved = false;
      cchess_board_set_piece(&game->board, 0, move.y1, rook);
      cchess_board_set_piece(&game->board, move.x2 + 1, move.y1, (cchess_piece_t) {CCHESS_COLOR_WHITE, CCHESS_PIECE_NONE, false});
    } else {
      printf("Error! Castle move data malformed in reverse_move!");
    }
    break;
  case CCHESS_MOVE_EN_PASSANT:
    cchess_board_set_piece(&game->board, move.x1, move.y1,
                           stored_move.moved_piece);
    cchess_board_set_piece(
        &game->board, move.x2, move.y2,
        (cchess_piece_t){CCHESS_COLOR_WHITE, CCHESS_PIECE_NONE, false});
    if ((move.y2) > (move.y1)) {
      cchess_board_set_piece(&game->board, move.x2, move.y2 - 1,
                             stored_move.captured_piece);
    } else if ((move.y2) < (move.y1)) {
      cchess_board_set_piece(&game->board, move.x2, move.y2 + 1,
                             stored_move.captured_piece);
    } else {
      printf("Error! en-passant move data malformed!");
    }
    break;
  }
}

bool cchess_game_is_checking(cchess_game_t *game) {

  cchess_move_t moves[MOVE_BUF_SIZE];
  size_t len = cchess_game_unfiltered_moves(game, moves);
  for (size_t i = 0; i < len; i++) {
    cchess_move_t move = moves[i];
    cchess_piece_t captured_piece =
        cchess_board_get_piece(&game->board, move.x2, move.y2);
    if (captured_piece.color == CCHESS_COLOR_OTHER_COLOR(game->current_color) &&
        captured_piece.type == CCHESS_PIECE_KING) {
      return true;
    }
  }
  return false;
}

bool cchess_game_is_in_check(cchess_game_t *game) {
  game->current_color = CCHESS_COLOR_OTHER_COLOR(game->current_color);
  bool check = cchess_game_is_checking(game);
  game->current_color = CCHESS_COLOR_OTHER_COLOR(game->current_color);
  return check;
}

bool valid_move(cchess_game_t *game, cchess_move_t move) {
  cchess_game_move(game, move);
  bool valid = !cchess_game_is_checking(game);
  cchess_game_reverse_move(game);
  return valid;
}

size_t cchess_game_moves(cchess_game_t *game, cchess_move_t *move_buf) {
  size_t len = cchess_game_unfiltered_moves(game, move_buf);
  size_t next_copy_index = 0;
  for (size_t i = 0; i < len; i++) {
    if (valid_move(game, move_buf[i])) {
      move_buf[next_copy_index] = move_buf[i];
      next_copy_index++;
    }
  }
  return next_copy_index;
}

cchess_game_t cchess_play_game(cchess_player_t *white, cchess_player_t *black) {
  cchess_game_t game;
  cchess_board_init(&game.board);
  game.moves = cchess_stored_move_stack_create();
  game.current_color = CCHESS_COLOR_WHITE;
  game.state = CCHESS_STATE_RUNNING;
  while (true) {
    if (white->out != NULL) {
      white->out(white, &game);
    }
    if (black->out != NULL) {
      black->out(black, &game);
    }

    cchess_move_t possible_moves[MOVE_BUF_SIZE];
    size_t moves_len = cchess_game_moves(&game, possible_moves);

    if (moves_len == 0) {
      if (cchess_game_is_in_check(&game)) {
        if (game.current_color == CCHESS_COLOR_WHITE) {
          game.state = CCHESS_STATE_BLACK_WON;
        } else {
          game.state = CCHESS_STATE_WHITE_WON;
        }

        return game; // checkmate, return winner
      }
      game.state = CCHESS_STATE_STALEMATE;
      return game;
    }
    cchess_move_t player_move;
    switch (game.current_color) {
    case CCHESS_COLOR_WHITE:
      player_move = white->in(white, possible_moves, moves_len);
      break;
    case CCHESS_COLOR_BLACK:
      player_move = black->in(black, possible_moves, moves_len);
      break;
    }

    cchess_game_move(&game, player_move);
  }
}
