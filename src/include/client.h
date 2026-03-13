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

  // Create a knowledge map: 0 = definitely safe, 1 = definitely mine, -1 = unknown
  int cell_knowledge[35][35];
  for (int i = 0; i < rows; i++) {
    for (int j = 0; j < columns; j++) {
      if (current_map[i][j] >= '0' && current_map[i][j] <= '8') {
        cell_knowledge[i][j] = 0;  // safe
      } else if (current_map[i][j] == '@') {
        cell_knowledge[i][j] = 1;  // mine
      } else {
        cell_knowledge[i][j] = -1;  // unknown
      }
    }
  }

  // Iteratively apply constraint reasoning
  bool changed = true;
  while (changed) {
    changed = false;

    for (int i = 0; i < rows; i++) {
      for (int j = 0; j < columns; j++) {
        if (current_map[i][j] >= '0' && current_map[i][j] <= '8') {
          int mine_count = current_map[i][j] - '0';
          int marked_neighbors = 0;
          int unvisited_neighbors = 0;
          int unvisited_list[8][2];
          int unvisited_count = 0;

          for (int k = 0; k < 8; k++) {
            int ni = i + dr[k];
            int nj = j + dc[k];
            if (ni >= 0 && ni < rows && nj >= 0 && nj < columns) {
              if (cell_knowledge[ni][nj] == 1) {
                marked_neighbors++;
              } else if (cell_knowledge[ni][nj] == -1) {
                unvisited_neighbors++;
                unvisited_list[unvisited_count][0] = ni;
                unvisited_list[unvisited_count][1] = nj;
                unvisited_count++;
              }
            }
          }

          int remaining_mines = mine_count - marked_neighbors;

          // If all remaining cells are mines, mark them
          if (remaining_mines == unvisited_neighbors && remaining_mines > 0) {
            for (int k = 0; k < unvisited_count; k++) {
              if (cell_knowledge[unvisited_list[k][0]][unvisited_list[k][1]] == -1) {
                cell_knowledge[unvisited_list[k][0]][unvisited_list[k][1]] = 1;
                changed = true;
              }
            }
          }

          // If no remaining mines, all cells are safe
          if (remaining_mines == 0 && unvisited_neighbors > 0) {
            for (int k = 0; k < unvisited_count; k++) {
              if (cell_knowledge[unvisited_list[k][0]][unvisited_list[k][1]] == -1) {
                cell_knowledge[unvisited_list[k][0]][unvisited_list[k][1]] = 0;
                changed = true;
              }
            }
          }
        }
      }
    }
  }

  // Strategy 1: Auto-explore where all mines are marked
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

        if (marked_neighbors == mine_count && unvisited_neighbors > 0) {
          Execute(i, j, 2);
          return;
        }
      }
    }
  }

  // Strategy 2: Mark identified mines
  for (int i = 0; i < rows; i++) {
    for (int j = 0; j < columns; j++) {
      if (current_map[i][j] == '?' && cell_knowledge[i][j] == 1) {
        Execute(i, j, 1);
        return;
      }
    }
  }

  // Strategy 3: Visit identified safe cells
  for (int i = 0; i < rows; i++) {
    for (int j = 0; j < columns; j++) {
      if (current_map[i][j] == '?' && cell_knowledge[i][j] == 0) {
        Execute(i, j, 0);
        return;
      }
    }
  }

  // Strategy 4: Probability-based guessing with better heuristics
  // Look for safest cell to guess based on multiple factors
  int best_r = -1, best_c = -1;
  int best_score = -1000;

  for (int i = 0; i < rows; i++) {
    for (int j = 0; j < columns; j++) {
      if (current_map[i][j] == '?') {
        int score = 0;
        int visible_neighbors = 0;
        int low_count_neighbors = 0;
        int high_count_neighbors = 0;

        // Count different types of neighbors
        for (int k = 0; k < 8; k++) {
          int ni = i + dr[k];
          int nj = j + dc[k];
          if (ni >= 0 && ni < rows && nj >= 0 && nj < columns) {
            if (current_map[ni][nj] >= '0' && current_map[ni][nj] <= '8') {
              visible_neighbors++;
              int num = current_map[ni][nj] - '0';

              // Count how many mines are already marked
              int marked = 0;
              int unknown = 0;
              for (int m = 0; m < 8; m++) {
                int nni = ni + dr[m];
                int nnj = nj + dc[m];
                if (nni >= 0 && nni < rows && nnj >= 0 && nnj < columns) {
                  if (current_map[nni][nnj] == '@') marked++;
                  if (current_map[nni][nnj] == '?') unknown++;
                }
              }

              int remaining = num - marked;
              if (remaining == 0) {
                score += 100;  // Very safe - all mines found
              } else if (unknown > 0 && remaining < unknown) {
                score += 10;  // Relatively safe
                low_count_neighbors++;
              } else if (unknown > 0 && remaining == unknown) {
                score -= 100;  // Dangerous - must be a mine
              } else {
                high_count_neighbors++;
              }
            }
          }
        }

        // Prefer cells with visible neighbors (not edges)
        score += visible_neighbors * 5;

        // Prefer cells with low mine count neighbors
        score += low_count_neighbors * 3;

        if (score > best_score) {
          best_score = score;
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