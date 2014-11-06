/*********************************************
 * routines to transform Sudokus
 * without changing validity
 * Code by M.Weller
 *********************************************
  */

#ifndef transform_cpp
#define transform_cpp

#include "transform.h"

// rotates coordinates (x,y) 90 degrees with rotation center (center2_x / 2, center2_y / 2)
void rotate90(int& x, int& y, const int center2_x, const int center2_y, const unsigned char times){
	int tmp;
	x = x*2 - center2_x;
	y = y*2 - center2_y;
	switch(times % 4){
		case 0: // rotate 0
			break;
		case 1: // rotate 90
			tmp = x;
			x = -y;
			y = tmp;
			break;
		case 2: // rotate 180
			x = -x;
			y = -y;
			break;
		case 3: // rotate -90 (=270)
			tmp = x;
			x = y;
			y = -tmp;
			break;
	}
	x = (x + center2_x) >> 1;
	y = (y + center2_y) >> 1;
}

void rotate90(sudoku* s, const unsigned char times){

}

void swap_content(sudoku_cell* cell1, sudoku_cell* cell2){
	uint tmp = cell1->get_content();
	cell1->set_content(cell2->get_content());
	cell2->set_content(tmp);
}

void swap_rows(sudoku* s, const uint row1, const uint row2){
	if(!s) return;
	uint digits = s->getnum_digits();
	if((row1 >= digits) || (row2 >= digits) || (row1 == row2)) return;

	for(uint x = 0; x < digits; x++){
		swap_content(s->get_cell(x, row1), s->get_cell(x, row2));
	}
}

void swap_boxrows(sudoku* s, const uint boxrow1, const uint boxrow2){
	if(!s) return;
	uint digits = s->getnum_digits();
	uint order = (uint)sqrt((double)digits);
	if(order * order != digits) return;
	if((boxrow1 >= order) || (boxrow2 >= order) || (boxrow1 == boxrow2)) return;

	for(uint i = 0; i < order; i++)
		swap_rows(s, (boxrow1 * order) + i, (boxrow2 * order) + i);
}

void transpose(sudoku* s){
	if(!s) return;
	uint digits = s->getnum_digits();

	for(uint x = 0; x < digits; x++)
		for(uint y = x + 1; y < digits; y++)
			swap_content(s->get_cell(x, y), s->get_cell(y, x));
}


void permute_digits(sudoku* s, const uint* new_digits){
	uint digits = s->getnum_digits();
	for(uint x = 0; x < digits; x++)
		for(uint y = 0; y < digits; y++)
			s->get_cell(x,y)->set_content(new_digits[s->get_cell(x,y)->get_content() - 1]);
}

void intswap(uint& x1, uint& x2){ uint tmp=x1;x1=x2;x2=tmp;}
bool is_ambigous_rect(sudoku* s, uint x0, uint y0, uint x1, uint y1){
	uint digits = s->getnum_digits();
	uint order = (uint)sqrt((double)digits);
	if((x0 == x1) || (y0 == y1)) return false;
	if(x0 > x1) intswap(x0,x1);
	if(y0 > y1) intswap(y0,y1);

	// the rect must intersect no more then 2 boxes
	if((x0 / order != x1 / order) && (y0 / order != y1 / order)) return false;
	// the digits across must be equal and nonzero
	uint cont00 = s->get_cell(x0,y0)->get_content();
	if(cont00 != s->get_cell(x1,y1)->get_content()) return false;
	uint cont01 = s->get_cell(x0,y1)->get_content();
	if(cont01 != s->get_cell(x1,y0)->get_content()) return false;
	if(!(cont00 && cont01)) return false;
	return true;
}

bool find_next_ambigous_rect(sudoku* s, uint& x0, uint& y0, uint& x1, uint& y1){
	uint digits = s->getnum_digits();
	y1++;
	while(!(is_ambigous_rect(s, x0, y0, x1, y1) || (x0 == digits))){
		y1++;
		if(y1 == digits) { y1 = y0; x1++;}
		if(x1 == digits) { x1 = x0; y0++;}
		if(y0 == digits) { y0 = 0; x0++;}
	}
	return (x0==digits);
}

void ambigous_rect(sudoku* s, uint x0, uint y0, uint x1, uint y1){
	if(x0 > x1) intswap(x0,x1);
	if(y0 > y1) intswap(y0,y1);

	if(!is_ambigous_rect(s, x0, y0, x1, y1)) return;

	// all right, then exchange
	uint cont00 = s->get_cell(x0,y0)->get_content();
	uint cont01 = s->get_cell(x0,y1)->get_content();
	s->get_cell(x0,y0)->set_content(cont01);
	s->get_cell(x0,y1)->set_content(cont00);
	s->get_cell(x1,y0)->set_content(cont00);
	s->get_cell(x1,y1)->set_content(cont01);
}

#endif

