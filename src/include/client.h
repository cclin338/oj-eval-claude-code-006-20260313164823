#ifndef CLIENT_H
#define CLIENT_H

#include <iostream>
#include <utility>

extern int rows;         // The count of rows of the game map.
extern int columns;      // The count of columns of the game map.
extern int total_mines;  // The count of mines of the game map.

// You MUST NOT use any other external variables except for rows, columns and total_mines.

// Additional global variables for client
char current_map[35][35];  // Current visible map state
int is_mine[35][35];       // -1: unknown, 0: safe, 1: mine
int visited_count;         // Count of visited grids
int marked_count;          // Count of marked mines

/**
 * @brief The definition of function Execute(int, int, bool)
 *
 * @details This function is designed to take a step when player the client's (or player's) role, and the implementation
 * of it has been finished by TA. (I hope my comments in code would be easy to understand T_T) If you do not understand
 * the contents, please ask TA for help immediately!!!
 *
 * @param r The row coordinate (0-based) of the block to be visited.
 * @param c The column coordinate (0-based) of the block to be visited.
 * @param type The type of operation to a certain block.
 * If type == 0, we'll execute VisitBlock(row, column).
 * If type == 1, we'll execute MarkMine(row, column).
 * If type == 2, we'll execute AutoExplore(row, column).
 * You should not call this function with other type values.
 */
void Execute(int r, int c, int type);

/**
 * @brief The definition of function InitGame()
 *
 * @details This function is designed to initialize the game. It should be called at the beginning of the game, which
 * will read the scale of the game map and the first step taken by the server (see README).
 */
void InitGame() {
  // Initialize all global variables
  for (int i = 0; i < rows; i++) {
    for (int j = 0; j < columns; j++) {
      current_map[i][j] = '?';
      is_mine[i][j] = -1;  // unknown
    }
  }
  visited_count = 0;
  marked_count = 0;

  int first_row, first_column;
  std::cin >> first_row >> first_column;
  Execute(first_row, first_column, 0);
}

/**
 * @brief The definition of function ReadMap()
 *
 * @details This function is designed to read the game map from stdin when playing the client's (or player's) role.
 * Since the client (or player) can only get the limited information of the game map, so if there is a 3 * 3 map as
 * above and only the block (2, 0) has been visited, the stdin would be
 *     ???
 *     12?
 *     01?
 */
void ReadMap() {
  for (int i = 0; i < rows; i++) {
    for (int j = 0; j < columns; j++) {
      std::cin >> current_map[i][j];

      // Update knowledge based on visible information
      if (current_map[i][j] >= '0' && current_map[i][j] <= '8') {
        is_mine[i][j] = 0;  // confirmed safe
      } else if (current_map[i][j] == '@') {
        is_mine[i][j] = 1;  // confirmed mine
      } else if (current_map[i][j] == 'X') {
        // Visited mine or marked non-mine (game should end)
        is_mine[i][j] = 1;
      }
    }
  }
}

/**
 * @brief The definition of function Decide()
 *
 * @details This function is designed to decide the next step when playing the client's (or player's) role. Open up your
 * mind and make your decision here! Caution: you can only execute once in this function.
 */
void Decide() {
  int dr[] = {-1, -1, -1, 0, 0, 1, 1, 1};
  int dc[] = {-1, 0, 1, -1, 1, -1, 0, 1};

  // Strategy 1: Check for auto-explore opportunities
  // If a visited cell has all its mines marked, auto-explore it
  for (int i = 0; i < rows; i++) {
    for (int j = 0; j < columns; j++) {
      if (current_map[i][j] >= '1' && current_map[i][j] <= '8') {
        int mine_count = current_map[i][j] - '0';
        int marked_neighbors = 0;
        int unvisited_neighbors = 0;

        for (int k = 0; k < 8; k++) {
          int ni = i + dr[k];
          int nj = j + dc[k];
          if (ni >= 0 && ni < rows && nj >= 0 && nj < columns) {
            if (current_map[ni][nj] == '@') {
              marked_neighbors++;
            } else if (current_map[ni][nj] == '?') {
              unvisited_neighbors++;
            }
          }
        }

        // If all mines around this cell are marked, we can auto-explore
        if (marked_neighbors == mine_count && unvisited_neighbors > 0) {
          Execute(i, j, 2);  // Auto-explore
          return;
        }
      }
    }
  }

  // Strategy 2: Mark obvious mines
  // If a visited cell has exactly as many unvisited neighbors as its mine count
  for (int i = 0; i < rows; i++) {
    for (int j = 0; j < columns; j++) {
      if (current_map[i][j] >= '1' && current_map[i][j] <= '8') {
        int mine_count = current_map[i][j] - '0';
        int marked_neighbors = 0;
        int unvisited_neighbors = 0;
        int first_unvisited_r = -1, first_unvisited_c = -1;

        for (int k = 0; k < 8; k++) {
          int ni = i + dr[k];
          int nj = j + dc[k];
          if (ni >= 0 && ni < rows && nj >= 0 && nj < columns) {
            if (current_map[ni][nj] == '@') {
              marked_neighbors++;
            } else if (current_map[ni][nj] == '?') {
              unvisited_neighbors++;
              if (first_unvisited_r == -1) {
                first_unvisited_r = ni;
                first_unvisited_c = nj;
              }
            }
          }
        }

        // If remaining unvisited neighbors equal remaining mines, mark them
        int remaining_mines = mine_count - marked_neighbors;
        if (remaining_mines > 0 && remaining_mines == unvisited_neighbors) {
          // Mark one of the unvisited neighbors as a mine
          Execute(first_unvisited_r, first_unvisited_c, 1);
          return;
        }
      }
    }
  }

  // Strategy 3: Visit safe cells
  // If a cell has all its mines already marked, visit an unvisited neighbor
  for (int i = 0; i < rows; i++) {
    for (int j = 0; j < columns; j++) {
      if (current_map[i][j] >= '0' && current_map[i][j] <= '8') {
        int mine_count = current_map[i][j] - '0';
        int marked_neighbors = 0;

        for (int k = 0; k < 8; k++) {
          int ni = i + dr[k];
          int nj = j + dc[k];
          if (ni >= 0 && ni < rows && nj >= 0 && nj < columns) {
            if (current_map[ni][nj] == '@') {
              marked_neighbors++;
            }
          }
        }

        // If all mines are marked, visit unvisited neighbors
        if (marked_neighbors == mine_count) {
          for (int k = 0; k < 8; k++) {
            int ni = i + dr[k];
            int nj = j + dc[k];
            if (ni >= 0 && ni < rows && nj >= 0 && nj < columns) {
              if (current_map[ni][nj] == '?') {
                Execute(ni, nj, 0);
                return;
              }
            }
          }
        }
      }
    }
  }

  // Strategy 4: If no safe move, take a guess on an unvisited cell
  // Try to find the cell with the most safe neighbors (best guess)
  int best_r = -1, best_c = -1;
  int best_score = -1;

  for (int i = 0; i < rows; i++) {
    for (int j = 0; j < columns; j++) {
      if (current_map[i][j] == '?') {
        int safe_neighbors = 0;
        for (int k = 0; k < 8; k++) {
          int ni = i + dr[k];
          int nj = j + dc[k];
          if (ni >= 0 && ni < rows && nj >= 0 && nj < columns) {
            if (current_map[ni][nj] >= '0' && current_map[ni][nj] <= '8') {
              safe_neighbors++;
            }
          }
        }

        if (safe_neighbors > best_score) {
          best_score = safe_neighbors;
          best_r = i;
          best_c = j;
        }
      }
    }
  }

  if (best_r != -1) {
    Execute(best_r, best_c, 0);
    return;
  }
}

#endif