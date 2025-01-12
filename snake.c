#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <ncurses.h>

#include "snake.h"

int main(void)
{
    srand(time(NULL));	    // seed random number generator

    initscr();		    // start curses mode
    cbreak();		    // disable line buffering
    noecho();		    // don't echo user input
    curs_set(0);	    // make cursor invisible
    keypad(stdscr, TRUE);   // enable arrow keys, etc.
    nodelay(stdscr, TRUE);  // don't wait for user input

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

void grid_delete(char** grid, int nrows)
{
    for (int row = 0; row < nrows; row++) {
	free(grid[row]);
    }

    free(grid);
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

void snake_delete(snake_t *snake)
{
    body_t *current = snake->head;
    while (current) {
	body_t *next = current->next;
	free(current);
	current = next;
    }

    free(snake);
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
    game->speed = 200;
    game->nrows = nrows;
    game->ncols = ncols;
    game->snake = snake;
    game->fruit = fruit;
    game->grid = grid;
    game->window = window;

    game_place_fruit(game);

    return game;
}

void game_delete(game_t *game)
{
    snake_delete(game->snake);
    grid_delete(game->grid, game->nrows);
    delwin(game->window);
    free(game->fruit);
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
	switch (getch()) {
	case KEY_UP:
	case 'W':
	case 'w':
	    if (delta != DOWN) delta = UP;
	    break;
	case KEY_DOWN:
	case 'S':
	case 's':
	    if (delta != UP) delta = DOWN;
	    break;
	case KEY_LEFT:
	case 'A':
	case 'a':
	    if (delta != RIGHT) delta = LEFT;
	    break;
	case KEY_RIGHT:
	case 'D':
	case 'd':
	    if (delta != LEFT) delta = RIGHT;
	    break;
	}

	game_tick(game, delta);
	game_print(game);
	usleep(game->speed * 1000);
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

    if (snake->row == game->fruit->row
	&& snake->col == game->fruit->col) {
	snake->size++;
	game->score += 100;
	game_place_fruit(game);
    } else {
	body_t *new_tail = snake->tail->prev;
	new_tail->next = NULL;
	game->grid[snake->tail->row][snake->tail->col] = ' ';
	free(snake->tail);
	snake->tail = new_tail;
    }
}

void game_print(game_t *game)
{
    // update score
    erase();
    mvprintw(0, 0, "Score: %d", game->score);

    // update game grid
    werase(game->window);
    box(game->window, 0, 0);

    mvwprintw(game->window, game->fruit->row, game->fruit->col, "@");

    body_t *current = game->snake->head;
    while (current) {
	mvwprintw(game->window, current->row, current->col, "X");
	current = current->next;
    }
    wrefresh(game->window);
}

void game_over(game_t *game)
{
    nodelay(stdscr, FALSE);

    char *message = "Game Over!";
    mvwprintw(game->window, game->nrows / 2,
	      (game->ncols - strlen(message)) / 2,
	      "%s", message);
    wrefresh(game->window);
    getch();
    game_delete(game);
    endwin();

    exit(0);
}
