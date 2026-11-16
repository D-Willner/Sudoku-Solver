#include "sudoku.h"
#include <stdio.h>
#include <string.h>

#define INVALID_MASK (1 << 15)

void print_board(sudoku_board_t board)
{
    for(int k = 0; k < 3; k++){
        for(int i = 0; i < 3; i++){
            for(int j = 0; j < 3; j++){
                printf("%d %d %d", board[3*k + i][3*j], board[3*k + i][3*j + 1], board[3*k + i][3*j + 2]);
                if(j!=2) printf(" | ");
            }
            printf("\n");
        }
        if(k!=2) printf("---------------------\n");
    }
}

static uint16_t check_vert(const sudoku_board_t board, int i, int j)
{
    if(board[i][j] != 0) return (1 << (board[i][j]-1));

    uint16_t ret = 0b0000000111111111;

    for(int k = 0; k < 9; k++){
        int val = board[i][k];
        if(val != 0) ret &= ~(1 << (val-1));
    }

    return ret;
}

static uint16_t check_hor(const sudoku_board_t board, int i, int j)
{
    if(board[i][j] != 0) return (1 << (board[i][j]-1));

    uint16_t ret = 0b0000000111111111;

    for(int k = 0; k < 9; k++){
        int val = board[k][j];
        if(val != 0) ret &= ~(1 << (val-1));
    }

    return ret;
}

static uint16_t check_box(const sudoku_board_t board, int i, int j)
{
    if(board[i][j] != 0) return (1 << (board[i][j]-1));

    uint16_t ret = 0b0000000111111111;

    int box_i = i/3;
    int box_j = j/3;

    for(int a = 0; a < 3; a++){
        for(int b = 0; b < 3; b++){
            int val = board[3*box_i + a][3*box_j + b];
            if(val != 0) ret &= ~(1 << (val-1));
        }
    }
    
    return ret;
}

void fill_poss(const sudoku_board_t board, sudoku_board_poss_t out)
{
    for(int i = 0; i < 9; i++){
        for(int j = 0; j < 9; j++){
            if(board[i][j] != 0) out[i][j] = 0;
            else {
                out[i][j] = check_vert(board, i, j) & check_hor(board, i, j) & check_box(board, i, j);
                if(out[i][j] == 0) out[i][j] = INVALID_MASK;
            }
        }
    }
}

static int is_invalid(const sudoku_board_poss_t filled_board)
{
    for(int i = 0; i < 9; i++){
        for(int j = 0; j < 9; j++){
            if(filled_board[i][j] & INVALID_MASK) return 1;
        }
    }

    return 0;
}

static int poss_amount(const sudoku_board_poss_t poss, int i, int j)
{
    int sum = 0;
    for(int k = 0; k < 9; k++){
        if(poss[i][j] & (1 << k)) sum++;
    }

    return sum;
}

static int mask_to_num(uint16_t mask)
{
    for(int i = 0; i < 9; i++){
        if(mask & (1<<i)) return i+1;
    }

    return 0;
}

static int is_solved(const sudoku_board_t board)
{
    for(int i = 0; i < 9; i++){
        for(int j = 0; j < 9; j++){
            if(board[i][j] == 0) return 0;
        }
    }

    return 1;
}

static int simple_solve_step(const sudoku_board_t board, sudoku_board_t out)
{
    memcpy(out, board, sizeof(int)*9*9);

    sudoku_board_poss_t poss = {0};
    fill_poss(board, poss);

    for(int i = 0; i < 9; i++){
        for(int j = 0; j < 9; j++){
            if(poss_amount(poss, i, j) == 1){
                out[i][j] = mask_to_num(poss[i][j]);
                return 0;
            }
        }
    }

    return 1;
}

// Basic idea:  For each square use the three basic rules (unique values on horizontal and vertical lines and in boxes)
//              to figure out which numbers could go in there. Then search for boxes with only one possibility 
//              and fill it in on the board. Continue until board is full or algorithm gets stuck.
// Improvement: Algorithm might get stuck on solvable boards. Use "recursive exploration" to solve those.
//              (Might be slow though, maybe better method exists) 
int simple_solve(const sudoku_board_t board, sudoku_board_t out)
{
    sudoku_board_t cur;
    
    memcpy(cur, board, sizeof(int)*9*9);
    int step = 1;
    int cont = 1;
    while(cont) {
        cont = !simple_solve_step(cur, out);

        printf("Working on sudoku: Step %d\n", step++);
        print_board(out);
        memcpy(cur, out, sizeof(int)*9*9);
    }

    return 0;
}

int rec_solve_step(const sudoku_board_t board, sudoku_board_t out, sudoku_board_poss_t poss)
{
    memcpy(out, board, sizeof(int)*9*9);

    fill_poss(board, poss);

    for(int i = 0; i < 9; i++){
        for(int j = 0; j < 9; j++){
            if(poss_amount(poss, i, j) == 1){
                out[i][j] = mask_to_num(poss[i][j]);
                return 0;
            } else if(poss[i][j] == INVALID_MASK) {
                return -1;
            }
        }
    }

    return 1;
}

// Idea: Like simple solve, but if it gets stuck it will recursively try out possible values for cells
//       Could be improved by not selecting the first cell for this, 
//       but instead selecting the cell with the lowest amount of possibilities
int rec_solve(const sudoku_board_t board, sudoku_board_t out)
{
    sudoku_board_t cur;
    sudoku_board_poss_t poss = {0};
    
    memcpy(cur, board, sizeof(int)*9*9);
    int cont = 0;
    while(cont == 0) {
        cont = rec_solve_step(cur, out, poss);
        if(cont == -1) return 1;

        memcpy(cur, out, sizeof(int)*9*9);
    }

    if(is_solved(cur)) return 0;

    // onion. could use helper function
    for(int i = 0; i < 9; i++){
        for(int j = 0; j < 9; j++){
            if(poss[i][j] != 0){
                for(int k = 0; k < 9; k++){
                    if(poss[i][j] & (1 << k)){
                        cur[i][j] = (k + 1);
                        int res = rec_solve(cur, out);
                        if(!res) return 0;

                        cur[i][j] = 0;
                    }
                }
                return 1;
            }
        }
    }

    return 1;
}