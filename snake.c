#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ncurses.h>

#include "snake.h"

int main(void)
{
    srand(time(NULL));	   // seed random number generator

    initscr();		   // start curses mode
    curs_set(0);	   // make cursor invisible
    noecho();		   // don't echo user input
    keypad(stdscr, TRUE);  // enable arrow keys, etc.

    int maxy;
    int maxx;
    getmaxyx(stdscr, maxy, maxx);

    int nrows = 20;
    int ncols = 40;
    int starty = (maxy - nrows) / 2;
    int startx = (maxx - ncols) / 2;

    WINDOW *window = newwin(nrows, ncols, starty, startx);

    game_t *game = game_init(nrows, ncols, window);
    game_print(game);
    game_loop(game);

    endwin();		   // end curses mode
    return 0;
}

char **grid_init(int nrows, int ncols)
{
    char **grid = malloc(sizeof(char *) * nrows);

    for (int row = 0; row < nrows; row++) {
	grid[row] = malloc(sizeof(char) * ncols);
    }

    for (int row = 0; row < nrows; row++) {
	for (int col = 0; col < ncols; col++) {
	    grid[row][col] = ' ';
	}
    }

    return grid;
}

body_t *body_init(int row, int col)
{
    body_t *body = malloc(sizeof(body_t));

    body->row = row;
    body->col = col;
    body->next = NULL;
    body->prev = NULL;

    return body;
}

snake_t *snake_init(int row, int col, int size)
{
    body_t *head = body_init(row, col);

    body_t *current = head;
    for (int i = 1; i < size; i++) {
	body_t *next = body_init(row - i, col);
	current->next = next;
	next->prev = current;
	current = next;
    }

    snake_t *snake = malloc(sizeof(snake_t));

    snake->row = row;
    snake->col = col;
    snake->size = size;
    snake->head = head;
    snake->tail = current;

    return snake;
}

fruit_t *fruit_init(int row, int col)
{
    fruit_t *fruit = malloc(sizeof(fruit));

    fruit->row = row;
    fruit->col = col;

    return fruit;
}

game_t *game_init(int nrows, int ncols, WINDOW *window)
{
    game_t *game = malloc(sizeof(game_t));
    snake_t *snake = snake_init(nrows/2, ncols/2, 3);
    fruit_t *fruit = fruit_init(0, 0);
    char **grid = grid_init(nrows, ncols);

    body_t *current = snake->head;
    while(current) {
	grid[current->row][current->col] = 'X';
	current = current->next;
    }

    game->score = 0;
    game->speed = 2;
    game->nrows = nrows;
    game->ncols = ncols;
    game->snake = snake;
    game->fruit = fruit;
    game->grid = grid;
    game->window = window;

    game_place_fruit(game);

    return game;
}

void game_place_fruit(game_t *game)
{
    int row, col;

    while (true) {
	row = rand() % game->nrows;
	col = rand() % game->ncols;
	if (game->grid[row][col] != 'X')
	    break;
    }

    game->fruit->row = row;
    game->fruit->col = col;
    game->grid[row][col] = '@';
}

void game_loop(game_t *game)
{
    delta_t delta = DOWN;

    while (true) {
	halfdelay(game->speed);

	switch (getch()) {
	case KEY_UP:
	    if (delta != DOWN) delta = UP;
	    break;
	case KEY_DOWN:
	    if (delta != UP) delta = DOWN;
	    break;
	case KEY_LEFT:
	    if (delta != RIGHT) delta = LEFT;
	    break;
	case KEY_RIGHT:
	    if (delta != LEFT) delta = RIGHT;
	    break;
	}

	game_tick(game, delta);
	game_print(game);
    }
}

void game_tick(game_t *game, delta_t delta)
{
    // create local var for convenience
    snake_t *snake = game->snake;

    switch (delta) {
    case UP:
	snake->row--;
	break;
    case DOWN:
	snake->row++;
	break;
    case LEFT:
	snake->col--;
	break;
    case RIGHT:
	snake->col++;
	break;
    }

    // check for wall collision
    if (snake->row < 0
	|| snake->row >= game->nrows
	|| snake->col < 0
	|| snake->col >= game->ncols) {
	game_over(game);
    }

    // check for snake collision
    if (game->grid[snake->row][snake->col] == 'X')
	game_over(game);

    body_t *new_head = body_init(snake->row, snake->col);
    snake->head->prev = new_head;
    new_head->next = snake->head;
    snake->head = new_head;
    game->grid[snake->row][snake->col] = 'X';

    // we eat some food
    if (snake->row == game->fruit->row
	&& snake->col == game->fruit->col) {
	snake->size++;
	game_place_fruit(game);
	return;
    }

    body_t *new_tail = snake->tail->prev;
    new_tail->next = NULL;
    game->grid[snake->tail->row][snake->tail->col] = ' ';
    free(snake->tail);
    snake->tail = new_tail;
}

void game_print(game_t *game)
{
    wclear(game->window);
    box(game->window, 0, 0);

    wmove(game->window, game->fruit->row, game->fruit->col);
    wprintw(game->window, "@");

    body_t *current = game->snake->head;
    while (current) {
	wmove(game->window, current->row, current->col);
	wprintw(game->window, "X");
	current = current->next;
    }
    wrefresh(game->window);
}

void game_over(game_t *game)
{
    cbreak();
    refresh();
    getch();
    endwin();

    exit(0);
}
