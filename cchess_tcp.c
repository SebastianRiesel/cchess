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

#include "cchess_tcp.h"
#include "cchess.h"
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

typedef struct {
    uint8_t x1;
    uint8_t y1;
    uint8_t x2;
    uint8_t y2;
    uint8_t type;
    uint8_t promotion_type;
} __attribute__((packed)) cchess_network_move_t;

cchess_move_t tcp_player_get_move(cchess_player_t *self, cchess_move_t *moves, size_t buf_len) {
    cchess_tcp_player_t* player = (cchess_tcp_player_t*) self;
    cchess_network_move_t net_move;
    cchess_move_t internal_move;

    memset(&internal_move, 0, sizeof(internal_move));

    if (player->sock < 0) {
        return internal_move; // Already disconnected
    }

    ssize_t bytes_received = recv(player->sock, &net_move, sizeof(net_move), MSG_WAITALL);

    if (bytes_received <= 0) {
        printf("Connection dropped by opponent.\n");
        close(player->sock);
        player->sock = -1;

        exit(-1);
    }

    internal_move.x1 = (size_t)net_move.x1;
    internal_move.y1 = (size_t)net_move.y1;
    internal_move.x2 = (size_t)net_move.x2;
    internal_move.y2 = (size_t)net_move.y2;
    internal_move.type = (cchess_move_type_t)net_move.type;
    internal_move.promotion_type = (cchess_piece_type_t)net_move.promotion_type;


    //TODO: check that the move is valid
    return internal_move;
}



void tcp_player_send_game(cchess_player_t *self, cchess_game_t *game) {
    cchess_tcp_player_t* player = (cchess_tcp_player_t*) self;
    if(game->current_color == player->my_color){
        return;
    }
    if(game->moves->size == 0){
        return;
    }

    cchess_move_t my_move = game->moves->moves[game->moves->size-1].move;

    cchess_network_move_t net_move;
    net_move.x1 = (uint8_t)my_move.x1;
    net_move.y1 = (uint8_t)my_move.y1;
    net_move.x2 = (uint8_t)my_move.x2;
    net_move.y2 = (uint8_t)my_move.y2;
    net_move.type = (uint8_t)my_move.type;
    net_move.promotion_type = (uint8_t)my_move.promotion_type;

    size_t move_size = sizeof(cchess_network_move_t);

    if(send(player->sock, &net_move, move_size, MSG_NOSIGNAL) == -1){
        close(player->sock);
        player->sock = -1;
        perror("Error while sending move");
        exit(-1);
    }
}

cchess_tcp_player_t* cchess_tcp_player_create(cchess_color_t color){
    cchess_tcp_player_t* player = (cchess_tcp_player_t*) malloc(sizeof(cchess_tcp_player_t));

    if(player == NULL) return NULL;

    player->player.in = tcp_player_get_move;
    player->player.out = tcp_player_send_game;

    player->sock = -1;
    player->my_color = color;

    return player;
}

void cchess_tcp_player_destroy(cchess_tcp_player_t* player){
    if (player == NULL)
        return;

    if (player->sock >= 0) {
        close(player->sock);
        player->sock = -1;
    }

    free(player);
}

int cchess_tcp_player_connect(cchess_tcp_player_t *player,char* address){

    if(player == NULL){
        return -1;
    }

    // other player address + port
    struct sockaddr_in sockaddr;
    memset(&sockaddr, 0, sizeof(sockaddr));
    sockaddr.sin_family = AF_INET;
    sockaddr.sin_port = htons (CCHESS_TCP_PORT);
    if (inet_aton(address, &sockaddr.sin_addr) == 0) {
        return -5;
    }


    if ((player->sock = socket (AF_INET, SOCK_STREAM, 0)) < 0){
        return -2;
    }
    printf ("Created socket.\n");

    if (connect ( player->sock, (struct sockaddr *) &sockaddr, sizeof (sockaddr)) != 0){
        close(player->sock);
        player->sock = -1;
        return -3;
    }
    printf ("Connected to %s\n", inet_ntoa (sockaddr.sin_addr));

    char other_color_char = player->my_color == CCHESS_COLOR_WHITE ? 'B' : 'W';

    if(send(player->sock, &other_color_char, 1, 0) != 1){
        close(player->sock);
        player->sock = -1;
        return -4;
    }

    printf("Notified other player that his color is %c\n", other_color_char);


    return 0;
}

int cchess_tcp_player_listen(cchess_tcp_player_t *player){
    int host_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    if(host_sock < 0){
        return -1;
    }

    printf("Created host socket.\n");

    // Fix: Allow immediate port reuse after a restart/crash
    int opt = 1;
    if (setsockopt(host_sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        close(host_sock);
        return -7;
    }

    struct sockaddr_in host;

    memset(&host, 0, sizeof(host));       /* Clear struct */
    host.sin_family = AF_INET;                  /* Internet/IP */
    host.sin_addr.s_addr = htonl(INADDR_ANY);   /* Incoming addr */
    host.sin_port = htons(CCHESS_TCP_PORT);       /* server port */

    if (bind(host_sock, (struct sockaddr *) &host, sizeof(host)) < 0) {
        close(host_sock);
        return -2;
    }

    if (listen(host_sock, 1) < 0) {
        close(host_sock);
        return -3;
    }

    struct sockaddr_in other_player_addr;
    unsigned int len = sizeof(other_player_addr);
    if ((player->sock = accept(host_sock, (struct sockaddr *) &other_player_addr, &len)) < 0) {
        close(host_sock);
        return -4;
    }
    close(host_sock);
    printf ("Connected to %s\n", inet_ntoa (other_player_addr.sin_addr));
    char my_color;

    if (recv(player->sock, &my_color, 1, 0) <= 0) {
        close(player->sock);
        player->sock = -1;
        return -5;
    }

    if(my_color == 'W'){
        player->my_color = CCHESS_COLOR_WHITE;
    }else if(my_color == 'B'){

        player->my_color = CCHESS_COLOR_BLACK;
    }else{
        close(player->sock);
        player->sock = -1;
        return -6;
    }

    return 0;
}


