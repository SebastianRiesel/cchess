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

#define CCHESS_TCP_PORT 38701
typedef struct{
    cchess_player_t player;
    int sock;
    cchess_color_t my_color;
} cchess_tcp_player_t;

cchess_tcp_player_t* cchess_tcp_player_create(cchess_color_t color);
void cchess_tcp_player_destroy(cchess_tcp_player_t* player);

int cchess_tcp_player_connect(cchess_tcp_player_t *player,char* address);
int cchess_tcp_player_listen(cchess_tcp_player_t *player);

