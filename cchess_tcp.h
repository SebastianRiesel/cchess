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

