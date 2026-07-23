#include "cchess_ai.h"
#include "cchess.h"
#include <float.h>
#include <stdlib.h>

cchess_move_t random_input(cchess_player_t *self, cchess_move_t *possible_moves,
                           size_t buf_len) {

  return possible_moves[((cchess_ai_random_player_t *)self)->randint(buf_len)];
}

cchess_ai_random_player_t *
cchess_ai_random_player_create(cchess_ai_randint_func f) {
  cchess_ai_random_player_t *player =
      (cchess_ai_random_player_t *)malloc(sizeof(cchess_ai_random_player_t));
  player->randint = f;
  player->player.in = random_input;
  player->player.out = NULL;
  return player;
}

void cchess_ai_random_player_destroy(cchess_ai_random_player_t *player) {
  free(player);
}

float cchess_ai_minimax_piece_score(cchess_game_t *game) {
  float score = 0.0;
  for (size_t x = 0; x < 8; x++) {
    for (size_t y = 0; y < 8; y++) {
      cchess_piece_t piece = cchess_board_get_piece(&game->board, x, y);
      float color_mult = piece.color == CCHESS_COLOR_WHITE ? 1 : -1;
      switch (piece.type) {
      case CCHESS_PIECE_NONE:
      case CCHESS_PIECE_KING:
        break;
      case CCHESS_PIECE_QUEEN:
        score += color_mult * 9;
        break;
      case CCHESS_PIECE_ROOK:
        score += color_mult * 5;
        break;
      case CCHESS_PIECE_KNIGHT:
      case CCHESS_PIECE_BISHOP:
        score += color_mult * 3;
        break;
      case CCHESS_PIECE_PAWN:
        score += color_mult * 1;
      }
    }
  }
  return score;
}

float max(float a, float b) { return a > b ? a : b; }

float min(float a, float b) { return a < b ? a : b; }

float minimax_player_get_score(cchess_ai_minimax_player_t *self,
                               cchess_game_t *game, float alpha, float beta,
                               uint8_t current_depth) {
  if (current_depth == self->depth) {
    return self->score_func(game);
  }

  cchess_move_t *possible_moves;
  size_t moves_len = cchess_game_moves(game, &possible_moves);

  if (moves_len == 0) {
    free(possible_moves);
    if (cchess_game_is_in_check(game)) {
      return game->current_color == CCHESS_COLOR_WHITE
                 ? -FLT_MAX + current_depth
                 : FLT_MAX - current_depth;
    }
    return 0;
  }
  float best_score;

  if (game->current_color == CCHESS_COLOR_WHITE) {
    best_score = -INFINITY;
  } else {
    best_score = INFINITY;
  }

  if (game->current_color == CCHESS_COLOR_WHITE) {
    for (size_t i = 0; i < moves_len; i++) {
      cchess_move_t move = possible_moves[i];
      cchess_game_move(game, move);
      float score =
          minimax_player_get_score(self, game, alpha, beta, current_depth + 1);
      cchess_game_reverse_move(game);
      best_score = max(score, best_score);
      alpha = max(alpha, best_score); // best thing that white can guarantee
      if (alpha >= beta) {
        break;
      }

    }
  } else {
    for (size_t i = 0; i < moves_len; i++) {
      cchess_move_t move = possible_moves[i];
      cchess_game_move(game, move);
      // this means white moved, this is whites branch
      float score =
          minimax_player_get_score(self, game, alpha, beta, current_depth + 1);
      cchess_game_reverse_move(game);
      best_score = min(score, best_score);
      beta = min(beta, best_score); // best thing that white can guarantee
      if (alpha >= beta) {
        break;
      }

    }
  }

  free(possible_moves);
  return best_score;
}

cchess_move_t minimax_player_get_move(cchess_player_t *self,
                                      cchess_move_t *moves, size_t buf_len) {
  cchess_ai_minimax_player_t *player = (cchess_ai_minimax_player_t *)self;
  cchess_game_t *game = player->current_game;
  cchess_move_t *possible_moves;
  size_t moves_len = cchess_game_moves(game, &possible_moves);

  if (moves_len == 0) {
    free(possible_moves);
    printf("Error: This point should not be reachable: %s at line %i", __FILE__,
           __LINE__);
    return (cchess_move_t){};
  }

  float best_score;
  cchess_move_t best_move;

  if (game->current_color == CCHESS_COLOR_WHITE) {
    best_score = -INFINITY;
  } else {
    best_score = INFINITY;
  }

  for (size_t i = 0; i < moves_len; i++) {
    cchess_move_t move = possible_moves[i];
    cchess_game_move(game, move);

    if (game->current_color == CCHESS_COLOR_BLACK) { // this means white moved
      float score =
          minimax_player_get_score(player, game, best_score, INFINITY, 1);
      if (score >= best_score) {
        best_score = score;
        best_move = move;
      }
    } else {
      float score =
          minimax_player_get_score(player, game, -INFINITY, best_score, 1);
      if (score <= best_score) {
        best_score = score;
        best_move = move;
      }
    }
    cchess_game_reverse_move(game);
  }

  free(possible_moves);
  return best_move;
}
void minimax_player_store_game(cchess_player_t *self, cchess_game_t *game) {
  cchess_ai_minimax_player_t *player = (cchess_ai_minimax_player_t *)self;
  player->current_game = game;
}

cchess_ai_minimax_player_t *
cchess_ai_minimax_player_create(uint8_t depth,
                                cchess_ai_score_func score_func) {
  cchess_ai_minimax_player_t *player =
      (cchess_ai_minimax_player_t *)malloc(sizeof(cchess_ai_minimax_player_t));
  player->depth = depth;
  player->score_func = score_func;
  player->player.in = minimax_player_get_move;
  player->player.out = minimax_player_store_game;
  return player;
}
void cchess_ai_minimax_player_destroy(cchess_ai_minimax_player_t *player) {
  free(player);
}
