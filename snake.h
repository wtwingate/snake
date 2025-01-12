#include <ncurses.h>

/* Data Structures */

typedef struct Body body_t;
struct Body {
    int row;	     // Y coordinate
    int col;	     // X coordinate
    body_t *next;    // pointer to next body segment
    body_t *prev;    // pointer to previous body segment
};

typedef struct Snake snake_t;
struct Snake {
    int row;	     // Y coordinate
    int col;	     // X coordinate
    int size;	     // number of body nodes
    body_t *head;    // pointer to first body segment
    body_t *tail;    // pointer to last body segment
};

typedef struct Fruit fruit_t;
struct Fruit {
    int row;	     // Y coordinate
    int col;	     // X coordinate
};

typedef struct Game game_t;
struct Game {
    int score;	     // total game score
    int speed;	     // game speed in milliseconds
    int nrows;	     // number of rows (Y)
    int ncols;	     // number of cols (X)
    snake_t *snake;  // pointer to Snake struct
    fruit_t *fruit;  // pointer to Fruit struct
    char **grid;     // grid layout
    WINDOW *window;  // game window
};

typedef enum Delta delta_t;
enum Delta {
    UP,
    DOWN,
    LEFT,
    RIGHT
};

/* Function Declarations */

char **grid_init(int nrows, int ncols);
void grid_delete(char** grid, int nrows);
body_t *body_init(int row, int col);
snake_t *snake_init(int row, int col, int size);
void snake_delete(snake_t *snake);
fruit_t *fruit_init(int row, int col);
game_t *game_init(int nrows, int ncols, WINDOW *window);
void game_delete(game_t *game);
void game_place_fruit(game_t *game);
void game_loop(game_t *game);
void game_tick(game_t *game, delta_t delta);
void game_print(game_t *game);
void game_over(game_t *game);
