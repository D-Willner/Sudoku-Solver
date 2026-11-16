CMAKE_MINIMUM_REQUIRED(VERSION 3.22)

PROJECT(sudoku_solver)

ADD_EXECUTABLE(sudoku_solver)

TARGET_SOURCES(sudoku_solver
PRIVATE
  main.c 
  sudoku.c
)