#include "cchess.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define ARR_INDEX(x, y) (y * 8 + x)

void cchess_board_clear(cchess_board_t *board) {
  for (size_t i = 0; i < 64; i++) {
    cchess_piece_t p = {CCHESS_COLOR_WHITE, CCHESS_PIECE_NONE, 0, 0};
    board->grid[i] = p;
  }
}

void cchess_board_init(cchess_board_t *board) {
  cchess_board_clear(board);

#define BGA(x, y) (board->grid[ARR_INDEX(x, y)])
#define CCP(color, type) ((cchess_piece_t){color, type, 0, 0})
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
  for (size_t x = 0; x < 8; x++) {
    BGA(x, 1) = CCP(CCHESS_COLOR_WHITE, CCHESS_PIECE_PAWN);
    BGA(x, 6) = CCP(CCHESS_COLOR_BLACK, CCHESS_PIECE_PAWN);
  }

#undef CCP
#undef BGA
}

cchess_piece_t cchess_board_get_piece(cchess_board_t *board, size_t x,
                                      size_t y) {

  return board->grid[ARR_INDEX(x, y)];
}

cchess_piece_t cchess_board_set_piece(cchess_board_t *board, size_t x, size_t y,
                                      cchess_piece_t piece) {
  cchess_piece_t p = board->grid[ARR_INDEX(x, y)];
  board->grid[ARR_INDEX(x, y)] = piece;
  return p;
}

cchess_piece_t cchess_board_raw_move(cchess_board_t *board, size_t x1,
                                     size_t y1, size_t x2, size_t y2) {
  cchess_piece_t p = board->grid[ARR_INDEX(x2, y2)];
  board->grid[ARR_INDEX(x2, y2)] = board->grid[ARR_INDEX(x1, y1)];
  board->grid[ARR_INDEX(x1, y1)] =
      (cchess_piece_t){CCHESS_COLOR_WHITE, CCHESS_PIECE_NONE, 0, 0};
  return p;
}

int cchess_move_to_string(cchess_move_t move, char *dest, size_t size) {
  int res;
  res =
      snprintf(dest, size, "cchess_normal_move_t[x1=%ld,y1=%ld,x2=%ld,y2=%ld]",
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
  for (ssize_t y = 7; y >= 0; y--) {
    if (size - n <= 2) {
      return n;
    }
    dest[n++] = ((char)y) + 49;
    dest[n++] = ' ';
    dest[n] = '\0';

    for (size_t x = 0; x < 8; x++) {
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
const ssize_t knight_x_offsets[] = {-1, 1, 2, 2, 1, -1, -2, -2};
const ssize_t knight_y_offsets[] = {2, 2, 1, -1, -2, -2, 1, -1};

// diagonal walking directions
const ssize_t diagional_x_directions[] = {1, 1, -1, -1};
const ssize_t diagional_y_directions[] = {1, -1, 1, -1};

// straight walking directions
const ssize_t straight_x_directions[] = {1, -1, 0, 0};
const ssize_t straight_y_directions[] = {0, 0, 1, -1};

#define APPEND_MOVE(move_buf, len, move)                                       \
  {                                                                            \
    move_buf[len] = move;                                                      \
    len++;                                                                     \
  }
#define M(x1, y1, x2, y2)                                                      \
  ((cchess_move_t){x1, y1, x2, y2, CCHESS_MOVE_NORMAL, CCHESS_PIECE_NONE})

size_t append_walk(cchess_board_t *board, cchess_color_t color, size_t x,
                   size_t y, ssize_t x_direction, ssize_t y_direction,
                   cchess_move_t *moves) {
  size_t buf_len = 0;
  ssize_t x2 = (ssize_t)x;
  ssize_t y2 = (ssize_t)y;
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

size_t cchess_board_unfiltered_moves(cchess_board_t *board,
                                     cchess_color_t color,
                                     cchess_move_t **moves) {

  // assume there are no positions, where more than 500 moves are possible
  cchess_move_t *move_buf = (cchess_move_t *)calloc(500, sizeof(cchess_move_t));
  size_t buf_len = 0;
  for (size_t x = 0; x < 8; x++) {
    for (size_t y = 0; y < 8; y++) {
      // size_t i = ARR_INDEX(x,y);
      cchess_piece_t piece = cchess_board_get_piece(board, x, y);
      if (piece.color != color) {
        continue;
      }
      int pawn_direction = color == CCHESS_COLOR_WHITE ? 1 : -1;
      size_t pawn_start_rank = color == CCHESS_COLOR_WHITE ? 1 : 6;
      size_t pawn_promotion_rank = color == CCHESS_COLOR_WHITE ? 7 : 0;
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
                neighbour_piece.pawn_is_en_passantable) {
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
                neighbour_piece.pawn_is_en_passantable) {
              cchess_move_t move = M(x, y, x - 1, y + pawn_direction);
              move.type = CCHESS_MOVE_EN_PASSANT;
              APPEND_MOVE(move_buf, buf_len, move);
            }
          }
        }
        break;
      case CCHESS_PIECE_KNIGHT:
        for (size_t k = 0; k < 8; k++) {

          ssize_t x2 = (ssize_t)x + knight_x_offsets[k];
          ssize_t y2 = (ssize_t)y + knight_y_offsets[k];

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
          ssize_t x_dir = diagional_x_directions[k];
          ssize_t y_dir = diagional_y_directions[k];
          size_t n = append_walk(board, color, x, y, x_dir, y_dir,
                                 (move_buf + buf_len));
          buf_len += n;
        }
        break;
      case CCHESS_PIECE_ROOK:
        for (size_t k = 0; k < 4; k++) {
          ssize_t x_dir = straight_x_directions[k];
          ssize_t y_dir = straight_y_directions[k];
          size_t n = append_walk(board, color, x, y, x_dir, y_dir,
                                 (move_buf + buf_len));
          buf_len += n;
        }
        break;
      case CCHESS_PIECE_QUEEN:
        for (size_t k = 0; k < 4; k++) {
          ssize_t x_dir = straight_x_directions[k];
          ssize_t y_dir = straight_y_directions[k];
          size_t n = append_walk(board, color, x, y, x_dir, y_dir,
                                 (move_buf + buf_len));
          buf_len += n;
        }
        for (size_t k = 0; k < 4; k++) {
          ssize_t x_dir = diagional_x_directions[k];
          ssize_t y_dir = diagional_y_directions[k];
          size_t n = append_walk(board, color, x, y, x_dir, y_dir,
                                 (move_buf + buf_len));
          buf_len += n;
        }
        break;
      case CCHESS_PIECE_KING:
        for (size_t k = 0; k < 4; k++) {
          ssize_t x_dir = straight_x_directions[k];
          ssize_t y_dir = straight_y_directions[k];
          ssize_t x2 = x + x_dir;
          ssize_t y2 = y + y_dir;
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
          ssize_t x_dir = diagional_x_directions[k];
          ssize_t y_dir = diagional_y_directions[k];
          ssize_t x2 = x + x_dir;
          ssize_t y2 = y + y_dir;
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
            for (size_t x2 = x - 1; x2 > 0; x2--) {
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
            for (size_t x2 = x + 1; x2 < 7; x2++) {
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
  *moves = move_buf;
  return buf_len;
}
#undef APPEND_MOVE

void cchess_board_move(cchess_board_t *board, cchess_move_t move) {

  // clear all en_passantable pawns
  for (size_t x = 0; x < 8; x++) {
    for (size_t y = 0; y < 8; y++) {
      cchess_piece_t p = cchess_board_get_piece(board, x, y);
      p.pawn_is_en_passantable = false;
      cchess_board_set_piece(board, x, y, p);
    }
  }

  cchess_piece_t piece = cchess_board_get_piece(board, move.x1, move.y1);

  // if pawn makes double move, it is en_passantable next turn
  if (move.type == CCHESS_MOVE_PAWN_DOUBLE) {
    piece.pawn_is_en_passantable = true;
  }

  piece.has_moved = true;

  // commit piece values onto board
  cchess_board_set_piece(board, move.x1, move.y1, piece);

  // make move
  cchess_board_raw_move(board, move.x1, move.y1, move.x2, move.y2);

  if (move.type == CCHESS_MOVE_PROMOTION) {
    cchess_board_set_piece(
        board, move.x2, move.y2,
        (cchess_piece_t){piece.color, move.promotion_type, true, false});
  }

  if (move.type == CCHESS_MOVE_CASTLE) {
    if (((ssize_t)move.x2) > ((ssize_t)move.x1)) {
      cchess_board_raw_move(board, 7, move.y1, move.x2 - 1, move.y1);
    } else if (((ssize_t)move.x2) < ((ssize_t)move.x1)) {
      cchess_board_raw_move(board, 0, move.y1, move.x2 + 1, move.y1);
    } else {
      printf("Error! Castle move data malformed!");
    }
  }

  if (move.type == CCHESS_MOVE_EN_PASSANT) {
    if (((ssize_t)move.y2) > ((ssize_t)move.y1)) {
      cchess_board_set_piece(board, move.x2, move.y2 - 1,
                             (cchess_piece_t){CCHESS_COLOR_WHITE,
                                              CCHESS_PIECE_NONE, false, false});
    } else if (((ssize_t)move.y2) < ((ssize_t)move.y1)) {
      cchess_board_set_piece(board, move.x2, move.y2 + 1,
                             (cchess_piece_t){CCHESS_COLOR_WHITE,
                                              CCHESS_PIECE_NONE, false, false});
    } else {
      printf("Error! en-passant move data malformed!");
    }
  }
}

bool cchess_board_color_is_in_check(cchess_board_t *board,
                                    cchess_color_t color) {
  cchess_color_t other_color = CCHESS_COLOR_OTHER_COLOR(color);

  cchess_move_t *moves;
  size_t len = cchess_board_unfiltered_moves(board, other_color, &moves);
  for (size_t i = 0; i < len; i++) {
    cchess_move_t move = moves[i];
    cchess_piece_t captured_piece =
        cchess_board_get_piece(board, move.x2, move.y2);
    if (captured_piece.color == color &&
        captured_piece.type == CCHESS_PIECE_KING) {
      free(moves);
      return true;
    }
  }
  free(moves);
  return false;
}

bool valid_move(cchess_board_t *board, cchess_color_t color,
                cchess_move_t move) {
  cchess_board_t board_copy = *board;
  cchess_board_move(&board_copy, move);
  return !cchess_board_color_is_in_check(&board_copy, color);
}

size_t cchess_board_moves(cchess_board_t *board, cchess_color_t color,
                          cchess_move_t **moves) {
  cchess_move_t *move_buf;
  size_t len = cchess_board_unfiltered_moves(board, color, &move_buf);
  size_t next_copy_index = 0;
  for (size_t i = 0; i < len; i++) {
    if (valid_move(board, color, move_buf[i])) {
      move_buf[next_copy_index] = move_buf[i];
      next_copy_index++;
    }
  }
  *moves = move_buf;
  return next_copy_index;
}

cchess_game_t cchess_play_game(cchess_player_t *white, cchess_player_t *black) {
  cchess_game_t game;
  cchess_board_init(&game.board);
  game.current_color = CCHESS_COLOR_WHITE;
  game.state = CCHESS_STATE_RUNNING;
  while (true) {
    if (white->out != NULL) {
      white->out(white, game);
    }
    if (black->out != NULL) {
      black->out(black, game);
    }

    cchess_move_t *possible_moves;
    size_t moves_len =
        cchess_board_moves(&game.board, game.current_color, &possible_moves);

    if (moves_len == 0) {
      if (cchess_board_color_is_in_check(&game.board, game.current_color)) {
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

    cchess_board_move(&game.board, player_move);

    game.current_color = CCHESS_COLOR_OTHER_COLOR(game.current_color);
    free(possible_moves);
  }
}
