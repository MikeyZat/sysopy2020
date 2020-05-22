#ifndef UTIL_H
#define UTIL_H

#define MAX_PLAYERS 15
#define MAX_BACKLOG 10
#define MAX_MESSAGE_LENGTH 256

typedef enum { EMPTY, O, X } board_field;

typedef struct {
  int is_o_turn;
  board_field fields[9];
} board_t;

board_t create_new_board() {
  board_t board = {1, {EMPTY}};
  return board;
}

int move(board_t* board, int position) {
  if (position < 0 || position > 9 || board->fields[position] != EMPTY)
    return 0;
  board->fields[position] = board->is_o_turn ? O : X;
  board->is_o_turn = !board->is_o_turn;
  return 1;
}

// returns either O or X if one of them win - returns EMPTY if no winner yet
board_field check_column(board_t* board) {
  for (int x = 0; x < 3; x++) {
    board_field first = board->fields[x];
    board_field second = board->fields[x + 3];
    board_field third = board->fields[x + 6];
    if (first == second && first == third && first != EMPTY) return first;
  }
  return EMPTY;
}

// returns either O or X if one of them win - returns EMPTY if no winner yet
board_field check_row(board_t* board) {
  for (int i = 0; i < 3; i++) {
    board_field first = board->fields[3 * i];
    board_field second = board->fields[3 * i + 1];
    board_field third = board->fields[3 * i + 2];
    if (first == second && first == third && first != EMPTY) return first;
  }
  return EMPTY;
}

// returns either O or X if one of them win - returns EMPTY if no winner yet
board_field check_diagonal(board_t* board) {
  board_field first = board->fields[3 * 0 + 0];
  board_field second = board->fields[3 * 1 + 1];
  board_field third = board->fields[3 * 2 + 2];
  if (first == second && first == third && first != EMPTY) return first;

  first = board->fields[3 * 0 + 2];
  second = board->fields[3 * 1 + 1];
  third = board->fields[3 * 2 + 1];
  if (first == second && first == third && first != EMPTY) return first;

  return EMPTY;
}

// returns either O or X if one of them win - returns EMPTY if no winner yet
board_field get_winner(board_t* board) {
  board_field column = check_column(board);
  board_field row = check_row(board);
  board_field diagonal = check_diagonal(board);

  if (column != EMPTY)
    return column;

  if (row != EMPTY)
    return row;

  return diagonal;
}

#endif
