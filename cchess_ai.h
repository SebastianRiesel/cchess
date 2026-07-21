#pragma once

#include "cchess.h"
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

typedef size_t (*cchess_ai_randint_func)(size_t max);

typedef struct {
  cchess_player_t player;
  cchess_ai_randint_func randint;
} cchess_ai_random_player_t;

cchess_ai_random_player_t *
cchess_ai_random_player_create(cchess_ai_randint_func f);

void cchess_ai_random_player_destroy(cchess_ai_random_player_t *player);

typedef float (*cchess_ai_score_func)(cchess_game_t *game);

float cchess_ai_minimax_piece_score(cchess_game_t *game);

typedef struct {
  cchess_player_t player;
  cchess_game_t* current_game;
  uint8_t depth;
  cchess_ai_score_func score_func;
} cchess_ai_minimax_player_t;

cchess_ai_minimax_player_t *
cchess_ai_minimax_player_create(uint8_t depth, cchess_ai_score_func score_func);
void cchess_ai_minimax_player_destroy(cchess_ai_minimax_player_t *player);
