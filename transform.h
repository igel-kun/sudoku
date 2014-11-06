/*********************************************
 * routines to transform Sudokus
 * without changing validity
 * Code by M.Weller
 *********************************************
 * Sudoku grids can be transformed without changing
 * their validity by
 * --geometric--
 * 1. swapping rows which intersect the same boxes
 * 2. swapping rows of boxes
 * 3. transposition
 * --numeric--
 * 4. permute digits
 * 5. flip an ambigous rectangle
 * and combinations of [1-3]:
 *  10. swapping columns which intersect the same boxes
 *  11. swapping columns of boxes
 *  12. flipping horizontally, vertically, diagonally
 *  13. rotating 90, 180, 270
 * i'm only implementing 1-5 since the others can be represented
 * as combinations of 1-3
 * --> find more transformations ! [the class of sudokus generatable from one
 *  								by only using these 6 rules is too small]
 */

#ifndef transform_h
#define transform_h

#include "sudoku.h"

void rotate90(int& x, int& y, const int center2_x, const int center2_y, const unsigned char times = 1);
void rotate90(sudoku* s, const unsigned char times = 1);
// geometric
void swap_rows(sudoku* s, const uint row1, const uint row2);
void swap_boxrows(sudoku* s, const uint boxrow1, const uint boxrow2);
void transpose(sudoku* s);
// numeric
void permute_digits(sudoku* s, const uint* new_digits);
bool find_next_ambigous_rect(sudoku* s, uint& _x0, uint& _y0, uint& _x1, uint& _y1);
void ambigous_rect(sudoku* s, uint x0, uint y0, uint x1, uint y1);
#endif

