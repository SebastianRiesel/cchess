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

#include "cchess_ai.h"
#include <SDL3/SDL_events.h>
#include <SDL3/SDL_init.h>
#include <SDL3/SDL_mutex.h>
#include <SDL3/SDL_rect.h>
#include <SDL3/SDL_render.h>
#include <SDL3/SDL_stdinc.h>
#include <SDL3/SDL_surface.h>
#include <SDL3/SDL_thread.h>
#include <SDL3/SDL_timer.h>
#define SDL_MAIN_USE_CALLBACKS 1 /* use the callbacks instead of main() */
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3_image/SDL_image.h>

#include "cchess.h"
#include "cchess_tcp.h"

#define FPS 60
#define TARGET_TICKS 1000 / FPS

#define WINDOW_WIDTH 1200
#define WINDOW_HEIGHT 900

#define SPRITE_EXTRA_RESOLUTION 10

#define BOARD_OFFSET 10.0f
#define FIELD_SIZE 100.0f

#define WHITE_FIELD_COLOR 0.8f, 0.8f, 0.6f, SDL_ALPHA_OPAQUE_FLOAT
#define BLACK_FIELD_COLOR 0.2f, 0.2f, 0.4f, SDL_ALPHA_OPAQUE_FLOAT
#define MARKED_FIELD_COLOR 0.9f, 0.1f, 0.1f, SDL_ALPHA_OPAQUE_FLOAT

#define FIELDS_INDEX(x, y) ((x) * 8 + (y))

typedef struct {
  SDL_Window *window;
  SDL_Renderer *renderer;

  Uint64 last_ticks;
  Uint64 delta_ticks;

  cchess_board_t current_board;
  SDL_Mutex *board_mutex;

  SDL_Mutex *input_mutex;
  int waiting_for_input;

  int pos1_selected; // bool
  size_t x1;
  size_t y1;

  size_t marked_size;
  size_t *x_marked;
  size_t *y_marked;

  int pos2_selected; // bool
  size_t x2;
  size_t y2;

  SDL_Texture *sprites[12];
  SDL_FRect fields[64];

  SDL_Thread *game_thread;
} AppState;

AppState *appstate;

void update_board(cchess_player_t *self, cchess_game_t *game) {
  SDL_LockMutex(appstate->board_mutex);
  appstate->current_board = game->board;
  SDL_UnlockMutex(appstate->board_mutex);
}

size_t randint(size_t max) { return SDL_rand(max); }

cchess_move_t wait_for_input(cchess_player_t *self,
                             cchess_move_t *possible_moves, size_t buf_len) {
  SDL_LockMutex(appstate->input_mutex);
  appstate->pos1_selected = 0;
  appstate->pos2_selected = 0;
  appstate->waiting_for_input = 1;

  cchess_move_t selected_move = {
      0, 0, 0, 0, CCHESS_MOVE_NORMAL, CCHESS_PIECE_NONE};
  int selected = 0;

  appstate->marked_size = 0;
  if (appstate->x_marked != NULL) {
    SDL_free(appstate->x_marked);
    SDL_free(appstate->y_marked);
  }
  appstate->x_marked = SDL_malloc(sizeof(size_t) * buf_len);
  appstate->y_marked = SDL_malloc(sizeof(size_t) * buf_len);

  SDL_UnlockMutex(appstate->input_mutex);

  while (!selected) {
    SDL_LockMutex(appstate->input_mutex);
    if (appstate->pos1_selected && !appstate->pos2_selected &&
        appstate->marked_size == 0) {
      SDL_Log("Selected pos1:%lu|%lu\n", appstate->x1, 7 - appstate->y1);
      for (size_t i = 0; i < buf_len; i++) {
        cchess_move_t move = possible_moves[i];

        if (move.x1 == appstate->x1 && move.y1 == 7 - appstate->y1) {
          appstate->x_marked[appstate->marked_size] = move.x2;
          appstate->y_marked[appstate->marked_size] = 7 - move.y2;
          appstate->marked_size++;
          SDL_Log("%u|%u -> %u|%u", move.x1, move.y1, move.x2, move.y2);
        }
      }
      if (appstate->marked_size == 0) {
        appstate->pos1_selected = 0;
        SDL_Log("Pos1 unselected because no moves from this position are "
                "possible, there are %lu possible moves\n",
                buf_len);
      }
    } else if (appstate->pos1_selected && appstate->pos2_selected) {
      for (size_t i = 0; i < buf_len; i++) {
        cchess_move_t move = possible_moves[i];
        if (move.x1 == appstate->x1 && move.y1 == 7 - appstate->y1 &&
            move.x2 == appstate->x2 && move.y2 == 7 - appstate->y2) {
          selected_move = move;
          SDL_Log("%u|%u -> %u|%u", selected_move.x1, selected_move.y1,
                  selected_move.x2, selected_move.y2);
          selected = 1;
          appstate->pos1_selected = 0;
          appstate->pos2_selected = 0;
          appstate->marked_size = 0;
        }
      }
      if (!selected) {
        appstate->marked_size = 0;
        appstate->pos2_selected = 0;
        appstate->x1 = appstate->x2;
        appstate->y1 = appstate->y2;
      }
    } else if (!appstate->pos1_selected && !appstate->pos2_selected) {
      appstate->marked_size = 0;
    }

    SDL_UnlockMutex(appstate->input_mutex);
  }

  SDL_LockMutex(appstate->input_mutex);
  appstate->waiting_for_input = 0;
  SDL_UnlockMutex(appstate->input_mutex);

  return selected_move;
}

typedef struct {
  cchess_color_t screen_color;
  cchess_player_t* other_player;
} start_game_data;

int start_game(void *data) {
  start_game_data* sgd= (start_game_data* ) data;
  cchess_player_t screen = {update_board, wait_for_input};
  //cchess_player_t black = {update_board, wait_for_input};

  cchess_player_t *other = sgd->other_player;

  cchess_game_t game;
  if(sgd->screen_color == CCHESS_COLOR_WHITE){
    game = cchess_play_game(&screen, other);
  }else{
    game = cchess_play_game(other, &screen);
  }

  SDL_free(data);

  char *result_str = game.state == CCHESS_STATE_STALEMATE   ? "stalemate"
                     : game.state == CCHESS_STATE_WHITE_WON ? "white"
                                                            : "black";
  printf("Result is: %s\n", result_str);
  return 0;
}

void render_board(AppState *as) {

  cchess_board_t board_copy;

  SDL_LockMutex(as->board_mutex);
  board_copy = as->current_board;
  SDL_UnlockMutex(as->board_mutex);

  SDL_SetRenderDrawColorFloat(as->renderer, WHITE_FIELD_COLOR);

  SDL_FRect rect =
      (SDL_FRect){BOARD_OFFSET, BOARD_OFFSET, FIELD_SIZE * 8, FIELD_SIZE * 8};
  SDL_RenderFillRect(as->renderer, &rect);

  SDL_SetRenderDrawColorFloat(as->renderer, BLACK_FIELD_COLOR);

  for (size_t x = 0; x < 8; x += 2) {
    for (size_t y = 0; y < 8; y++) {
      SDL_FRect rect =
          as->fields[FIELDS_INDEX(x + y % 2, y)]; // makes checkerboard pattern
      SDL_RenderFillRect(as->renderer, &rect);
    }
  }

  SDL_LockMutex(as->input_mutex);
  SDL_SetRenderDrawColorFloat(as->renderer, MARKED_FIELD_COLOR);
  for (size_t i = 0; i < as->marked_size; i++) {
    SDL_FRect rect = as->fields[FIELDS_INDEX(as->x_marked[i], as->y_marked[i])];
    SDL_RenderFillRect(as->renderer, &rect);
  }

  SDL_UnlockMutex(as->input_mutex);

  for (size_t x = 0; x < 8; x++) {
    for (size_t y = 0; y < 8; y++) {
      SDL_FRect dst = as->fields[FIELDS_INDEX(x, y)];
      SDL_FRect src = {0, 0, FIELD_SIZE * SPRITE_EXTRA_RESOLUTION,
                       FIELD_SIZE * SPRITE_EXTRA_RESOLUTION};
      cchess_piece_t piece = cchess_board_get_piece(&board_copy, x, 7 - y);
      if (piece.type == CCHESS_PIECE_NONE)
        continue;

      size_t sprite_index = 0;
      if (piece.color == CCHESS_COLOR_BLACK) {
        sprite_index += 6;
      }
      switch (piece.type) {
      case CCHESS_PIECE_KING:
        break;
      case CCHESS_PIECE_QUEEN:
        sprite_index += 1;
        break;
      case CCHESS_PIECE_ROOK:
        sprite_index += 2;
        break;
      case CCHESS_PIECE_BISHOP:
        sprite_index += 3;
        break;
      case CCHESS_PIECE_KNIGHT:
        sprite_index += 4;
        break;
      case CCHESS_PIECE_PAWN:
        sprite_index += 5;
        break;
      case CCHESS_PIECE_NONE:
        break; // unreachable
      }
      SDL_RenderTexture(as->renderer, as->sprites[sprite_index], &src, &dst);
    };
  }
}

#define LOAD_SPRITE(appstate, file, index)                                     \
  {                                                                            \
    SDL_IOStream *io = SDL_IOFromFile("graphics/" file, "rb");                 \
    SDL_Surface *surface =                                                     \
        IMG_LoadSizedSVG_IO(io, FIELD_SIZE * SPRITE_EXTRA_RESOLUTION,          \
                            FIELD_SIZE * SPRITE_EXTRA_RESOLUTION);             \
    SDL_Texture *texture =                                                     \
        SDL_CreateTextureFromSurface(appstate->renderer, surface);             \
    appstate->sprites[index] = texture;                                        \
    SDL_DestroySurface(surface);                                               \
    SDL_CloseIO(io);                                                           \
  }

void load_sprites(AppState *as) {
  LOAD_SPRITE(as, "white_king.svg", 0);
  LOAD_SPRITE(as, "white_queen.svg", 1);
  LOAD_SPRITE(as, "white_rook.svg", 2);
  LOAD_SPRITE(as, "white_bishop.svg", 3);
  LOAD_SPRITE(as, "white_knight.svg", 4);
  LOAD_SPRITE(as, "white_pawn.svg", 5);
  LOAD_SPRITE(as, "black_king.svg", 6);
  LOAD_SPRITE(as, "black_queen.svg", 7);
  LOAD_SPRITE(as, "black_rook.svg", 8);
  LOAD_SPRITE(as, "black_bishop.svg", 9);
  LOAD_SPRITE(as, "black_knight.svg", 10);
  LOAD_SPRITE(as, "black_pawn.svg", 11);
}

void print_usage(char* name){
  printf("Usage:\n%s <mode> <args>\n", name);
  printf("Modes:\n\tlocal\n\tlocal_vs_bot\n\thost\n\tconnect <ip_addr>\n");
}

/* This function runs once at startup. */
SDL_AppResult SDL_AppInit(void **appstate_ptr, int argc, char *argv[]) {
  SDL_SetAppMetadata("CChess", "1.0", "de.sriesel.cchess");

  if (!SDL_Init(SDL_INIT_VIDEO)) {
    SDL_Log("Couldn't initialize SDL: %s", SDL_GetError());
    return SDL_APP_FAILURE;
  }

  AppState *state = (AppState *)SDL_malloc(sizeof(AppState));
  *appstate_ptr = state;

  if (!SDL_CreateWindowAndRenderer("CChess", WINDOW_WIDTH, WINDOW_HEIGHT,
                                   SDL_WINDOW_RESIZABLE, &(state->window),
                                   &(state->renderer))) {
    SDL_Log("Couldn't create window/renderer: %s", SDL_GetError());
    return SDL_APP_FAILURE;
  }
  SDL_SetRenderLogicalPresentation(state->renderer, WINDOW_WIDTH, WINDOW_HEIGHT,
                                   SDL_LOGICAL_PRESENTATION_LETTERBOX);

  // init values
  state->last_ticks = 0;
  state->delta_ticks = 0;
  cchess_board_init(&state->current_board);

  for (size_t x = 0; x < 8; x++) {
    for (size_t y = 0; y < 8; y++) {
      state->fields[FIELDS_INDEX(x, y)] =
          (SDL_FRect){BOARD_OFFSET + x * FIELD_SIZE,
                      BOARD_OFFSET + y * FIELD_SIZE, FIELD_SIZE, FIELD_SIZE};
    }
  }

  load_sprites(state);

  state->waiting_for_input = 0;
  state->input_mutex = SDL_CreateMutex();
  state->pos1_selected = 0;
  state->pos2_selected = 0;
  state->marked_size = 0;
  state->x_marked = NULL;
  state->y_marked = NULL;

  appstate = state;

  state->board_mutex = SDL_CreateMutex();

  cchess_tcp_player_t* tcp_player = cchess_tcp_player_create(CCHESS_COLOR_WHITE);

  if (tcp_player == NULL) {
    fprintf(stderr, "Failed to allocate TCP player.\n");
    return SDL_APP_FAILURE;
  }

  if(argc == 1){
    printf("No arguments provided.");
    print_usage(argv[0]);
    return SDL_APP_FAILURE;
  }

  char* mode = argv[1];
  start_game_data *data = SDL_malloc(sizeof(start_game_data));
  *data = (start_game_data){CCHESS_COLOR_WHITE, NULL};

  if(strcmp(mode,"local") == 0){
    cchess_player_t* screen = (cchess_player_t*)SDL_malloc(sizeof(cchess_player_t));
    *screen = (cchess_player_t){update_board, wait_for_input};
    data->other_player = screen;
    data->screen_color = CCHESS_COLOR_WHITE;

  }else if(strcmp(mode,"local_vs_bot") == 0){
    cchess_player_t* bot = (cchess_player_t*)cchess_ai_minimax_player_create(5,cchess_ai_minimax_piece_score);
    data->other_player = bot;
    data->screen_color = CCHESS_COLOR_WHITE;
  }else if(strcmp(mode, "host") == 0){
    printf("Host mode: Listening for incoming connections on port %d...\n", CCHESS_TCP_PORT);

    int status = cchess_tcp_player_listen(tcp_player);
    if (status != 0) {
      fprintf(stderr, "Failed to set up host or accept connection. Error code: %d\n", status);
      cchess_tcp_player_destroy(tcp_player);
      return SDL_APP_FAILURE;
    }

    printf("Player connected! Assigned color: %s.\n",
           tcp_player->my_color == CCHESS_COLOR_WHITE ? "White" : "Black");

    data->screen_color = tcp_player->my_color;
    data->other_player = (cchess_player_t*)tcp_player;
  }else if(strcmp(mode, "connect") == 0){
    if(argc < 2){
      printf("Error: Please provide IP Adress of hosting player.");
      return SDL_APP_FAILURE;

    }

    char* host_ip = argv[2];
    printf("Client mode: Attempting to connect to %s...\n", host_ip);

    int status = cchess_tcp_player_connect(tcp_player, host_ip);
    if (status != 0) {
      fprintf(stderr, "Connection failed with error code: %d\n", status);
      cchess_tcp_player_destroy(tcp_player);
      return SDL_APP_FAILURE;
    }

    printf("Successfully connected! Playing as %s.\n",
           tcp_player->my_color == CCHESS_COLOR_WHITE ? "White" : "Black");

    data->screen_color = tcp_player->my_color;
    data->other_player = (cchess_player_t*) tcp_player;

  }else{

    printf("Error: Please provide a correct mode.");
    print_usage(argv[0]);
    return SDL_APP_FAILURE;

  }

  state->game_thread = SDL_CreateThread(start_game, "game_thread", (void*)data);

  return SDL_APP_CONTINUE;
}

void clicked_on_square(AppState *state, size_t x, size_t y) {
  SDL_LockMutex(state->input_mutex);
  if (state->waiting_for_input) {
    if (!state->pos1_selected) {
      state->x1 = x;
      state->y1 = y;
      state->pos1_selected = 1;

    } else if (!state->pos2_selected) {
      state->x2 = x;
      state->y2 = y;
      state->pos2_selected = 1;
    }
  }
  SDL_UnlockMutex(state->input_mutex);
}

/* This function runs when a new event (mouse input, keypresses, etc) occurs.
 */
SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event) {
  AppState *as = (AppState *)appstate;
  if (event->type == SDL_EVENT_QUIT) {
    return SDL_APP_SUCCESS; /* end the program, reporting success to the OS.
                             */
  }

  SDL_ConvertEventToRenderCoordinates(as->renderer, event);

  if (event->type == SDL_EVENT_MOUSE_BUTTON_DOWN) {
    SDL_MouseButtonEvent mouse_event = event->button;
    size_t x = (size_t)(mouse_event.x - BOARD_OFFSET) / FIELD_SIZE;
    size_t y = (size_t)(mouse_event.y - BOARD_OFFSET) / FIELD_SIZE;
    if (x < 8 && y < 8) {
      clicked_on_square(as, x, y);
    }
  }
  return SDL_APP_CONTINUE; /* carry on with the program! */
}

/* This function runs once per frame, and is the heart of the program. */
SDL_AppResult SDL_AppIterate(void *appstate) {
  AppState *as = (AppState *)appstate;

  SDL_SetRenderDrawColorFloat(
      as->renderer, 0.0f, 0.0f, 0.0f,
      SDL_ALPHA_OPAQUE_FLOAT); /* new color, full alpha. */

  /* clear the window to the draw color. */
  SDL_RenderClear(as->renderer);

  SDL_SetRenderDrawColorFloat(
      as->renderer, 1.0f, 1.0f, 1.0f,
      SDL_ALPHA_OPAQUE_FLOAT); /* new color, full alpha. */

  SDL_FRect rect = {0, 0, WINDOW_WIDTH, WINDOW_HEIGHT};
  SDL_RenderRect(as->renderer, &rect);

  render_board(as);
  /* put the newly-cleared rendering on the screen. */
  SDL_RenderPresent(as->renderer);

  Uint64 current_ticks = SDL_GetTicks();
  Uint64 delta_ticks = 0;
  if (as->last_ticks == 0) {
    as->last_ticks = current_ticks;
  }
  delta_ticks = current_ticks - as->last_ticks;

  if (delta_ticks < TARGET_TICKS) {
    SDL_Delay(TARGET_TICKS - delta_ticks);
  }

  current_ticks = SDL_GetTicks();
  as->delta_ticks = current_ticks - as->last_ticks;
  as->last_ticks = current_ticks;
  // SDL_Log("%lu\n", as->delta_ticks);
  return SDL_APP_CONTINUE;
}
/* This function runs once at shutdown. */
void SDL_AppQuit(void *appstate, SDL_AppResult result) {
  AppState *as = (AppState *)appstate;
  for (size_t i = 0; i < 12; i++) {
    SDL_DestroyTexture(as->sprites[i]);
  }

  SDL_DestroyMutex(as->board_mutex);
  SDL_DestroyRenderer(as->renderer);
  SDL_DestroyWindow(as->window);
  SDL_free(as);
}
