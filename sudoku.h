#pragma once
#include <stdint.h>

typedef int sudoku_board_t[9][9];

typedef uint16_t sudoku_board_poss_t[9][9];

void print_board(sudoku_board_t board);

void fill_poss(const sudoku_board_t board, sudoku_board_poss_t out);

int simple_solve(const sudoku_board_t board, sudoku_board_t out);

int rec_solve(const sudoku_board_t board, sudoku_board_t out);