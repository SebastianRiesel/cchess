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
  size_t count;
} cchess_ai_minimax_player_t;

cchess_ai_minimax_player_t *
cchess_ai_minimax_player_create(uint8_t depth, cchess_ai_score_func score_func);
void cchess_ai_minimax_player_destroy(cchess_ai_minimax_player_t *player);
