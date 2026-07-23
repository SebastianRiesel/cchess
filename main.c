#include "cchess.h"
#include <stdio.h>
#include <stdlib.h>

#define BOARD_STR_SIZE 500

cchess_move_t get_move_from_terminal() {
  size_t x1, y1, x2, y2;
  while (true) {
    printf("Please input your move (x1,y1|x2,y2):");
    size_t parsed_args = scanf("%lu,%lu|%lu,%lu", &x1, &y1, &x2, &y2);
    if (parsed_args != 4) {
      printf("Error while parsing youre move, please retry!");
      continue;
    }
    break;
  }
  return (cchess_move_t){
      x1 - 1, y1 - 1, x2 - 1, y2 - 1, CCHESS_MOVE_NORMAL, CCHESS_PIECE_QUEEN};
}

cchess_move_t get_possible_move_from_terminal(cchess_player_t *self,
                                              cchess_move_t *moves,
                                              size_t move_len) {
  while (true) {
    cchess_move_t m1 = get_move_from_terminal();
    for (size_t i = 0; i < move_len; i++) {
      cchess_move_t m2 = moves[i];
      if (m1.x1 == m2.x1 && m1.y1 == m2.y1 && m1.x2 == m2.x2 &&
          m1.y2 == m2.y2) {
        return m2;
      }
    }
    printf("This move is not possible!");
  }
}

void print_board(cchess_player_t *self, cchess_game_t* game) {
  char buf[BOARD_STR_SIZE];
  cchess_board_to_string(&game.board, buf, BOARD_STR_SIZE - 1);
  printf("%s", buf);
}

int main() {

  cchess_player_t white = {print_board, get_possible_move_from_terminal};
  cchess_player_t black = {print_board, get_possible_move_from_terminal};

  cchess_game_t game = cchess_play_game(&white, &black);

  char *result_str = game.state == CCHESS_STATE_STALEMATE   ? "stalemate"
                     : game.state == CCHESS_STATE_WHITE_WON ? "white"
                                                            : "black";
  printf("Result is: %s\n", result_str);
  return 0;
}
