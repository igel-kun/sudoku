/*****************************************
 * general header file for sudoku puzzles
 * and rules wrapper
 * by M.Weller
 ****************************************/
#ifndef sudoku_h
#define sudoku_h

#include <math.h>
#include <iostream>
#include <set>
#include <vector>
#include "stdio.h"
#include "stdlib.h"
#include "string.h"

#define DEBUG 0
#define MAX_DIGITS 1024
#define dbgout if(DEBUG) cout
#define dbgprint if(DEBUG) printf
#define diewith(x)  {cout<< x; exit(1);}
#define uint unsigned int
#define byte unsigned char

#define EQ_PERMUTE_DIGITS	1
#define EQ_ROTATE			2
#define EQ_FLIP				4
#define EQ_TRANSPOSE		8

#define EQ_NONGEOMETRIC		EQ_PERMUTE_DIGITS
#define EQ_GEOMETRIC		EQ_ROTATE | EQ_FLIP | EQ_TRANSPOSE
#define EQ_ALL				EQ_GEOMETRIC | EQ_NONGEOMETRIC

using namespace std;

class sudoku;

// rules wrapper class
// pass an object of this class to sudoku::applyrule() to
// apply the rule to a sudoku
class vtree_node;

class sudoku_cell{
protected:
	uint num_digits;
	uint content;
	uint x;
	uint y;

	void init(const uint _num_digits, const uint _content, const uint _x, const uint _y);
public:
	// constructor
	sudoku_cell(const uint _num_digits, const uint _x, const uint _y, const uint _content = 0);
	// copy constructor
	sudoku_cell(const sudoku_cell& cell);
	// destructor
	~sudoku_cell();
	// user interface
	uint get_content() const;
	void set_content(const uint _content);
	void remove_content();
	uint get_num_digits()const;
	uint get_x() const;
	uint get_y() const;
 
  operator int() const {
    return y * num_digits + x;
  }
  friend ostream& operator<<(ostream& os, const sudoku_cell& s){
    os << "(" << s.x << "," << s.y << ")";
    if(s.content) os << "[" << s.content << "]";
    return os;
  }
};
// sudoku-grid wrapper
class sudoku {
protected:
	uint num_digits;
	vector<sudoku_cell*>* grid;

	uint get_index(const uint x, const uint y) const;
	void init(const uint digits, const vector<sudoku_cell*>* copy_grid = NULL);
	char* read_first_line(const char* filename);
	void mem_free();

public:
	sudoku() {};
	// constructor
	sudoku(const uint digits);
	// construct from a file
	sudoku(const char* filename, const bool read_field = true);
	// copy constructor
	sudoku(const sudoku& s);
	// destructor
	~sudoku();
	// write a cell to a position in the grid
	bool put_cell(const uint x, const uint y, sudoku_cell* cell);
	// read a cell from a position in the grid
	sudoku_cell* get_cell(const uint x, const uint y) const;
	// return the squared order [ = how many different digits there are]
	uint getnum_digits() const;
	// return a set of cells in row x
	void getrow(const uint x, set<sudoku_cell*>* group) const;
	// return a set of cells in the column y
	void getcolumn(const uint y, set<sudoku_cell*>* group) const;
	// return a set of cells in the square n
	void getsquare(const uint n, set<sudoku_cell*>* group) const;
	// get the [row, col, square] (dependent on group_nr) of the cell at (x,y)
	set<sudoku_cell*>* getgroup(const uint x, const uint y, const uint group_nr) const;
	// return a set of cells in the same row as cell (x,y)
	void getrow(const uint x, const uint y, set<sudoku_cell*>* group) const;
	// return a set of cells in the same column as cell (x,y)
	void getcolumn(const uint x, const uint y, set<sudoku_cell*>* group) const;
	// return a set of cells in the same square as cell (x,y)
	void getsquare(const uint x, const uint y, set<sudoku_cell*>* group) const;
	// get the n'th [row, col, square] (dependent on group_nr) 
	set<sudoku_cell*>* getgroup(const uint n, const uint group_nr) const;
	// read a sudoku field from a given file with the first line provided in first
	void read_from_file(const char* filename, const char* first = NULL);
	// output the grid to the stardard output stream
	void print()const;
	bool is_valid()const;
	// equality ono several levels [see above]
	bool is_equal(const sudoku& s, const uint levels) const;
	// equality comparing on level 0
	bool operator==(const sudoku& s) const;
	friend ostream& operator<<(ostream& os, const sudoku& s);
	friend istream& operator>>(istream& is, sudoku& s);
};


#endif
