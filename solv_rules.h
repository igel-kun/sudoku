/*********************************************
 * Ruleset for sudoku-solution
 * Rules by D.Eppstein
 * Code by M.Weller
 *********************************************

Rules are:
	flood
	eliminate
	locate
	align
	subproblem
	digit
[!]	transitive chain analysis

 Rules return true iff they could be applied to the given cell
*/

#ifndef solv_rules_h
#define solv_rules_h

#include "fptree.h"
#include "sudoku.h"

// each rule has a level specified here
#define LVL_FLOOD     (1<<0)
#define LVL_ELIMINATE (1<<1)
#define LVL_LOCATE    (1<<2)
#define LVL_GROUP     (1<<4)
#define LVL_ALIGN     (1<<5)
#define LVL_ALL       ((1<<6)-1)
// returns if level x is allowed with level_bits y [or vice versa, its symmetric]
#define level_allowed(x,y) ((x)&(y))
class solv_sudoku;

class solv_rule {
private:
	// location of the most recent application of this rule
	uint lastx;
	uint lasty;
	bool (*apply_func)(const uint x, const uint y, solv_sudoku*, const uint);

	bool __apply(const uint x, const uint y, solv_sudoku* s, const uint level_bits) const;
public:
	solv_rule(bool (*apply_f)(const uint x, const uint y, solv_sudoku*, const uint));
	// apply rule at a given location and return success [validity]
	bool apply(const uint x, const uint y, solv_sudoku* s, const uint level_bits) const;
	// find a suitable location and apply rule there, return whether the
	// rule could be applied [rule is applied just once!]
	bool apply(solv_sudoku* s, const uint level_bits);
	~solv_rule();
};

// a sudoku_cell with constraint propagation capabilities
class solv_cell{
protected:
	vector<fp_node*>* nodes;
	sudoku_cell* cell;
  uint num_digits;

	void sinit(const uint _num_digits);
	bool remove_thesis(const int digit, const uint level_bits);
public:
	solv_cell(const uint _num_digits, const uint _x, const uint _y, const uint _content = 0);
	solv_cell(sudoku_cell* _cell);
	solv_cell(const solv_cell& _scell);
	~solv_cell();
	// something special: indices go from -num_digits to num_digits
	// however: 0 is invalid
	fp_node*& operator[](const int digit);
	fp_node* const& operator[](const int digit) const;
	uint get_content() const;
	void set_content(const uint digit, const uint level_bits);
	void remove_content();
	uint get_x() const;
	uint get_y() const;
	uint count_poss() const;
  uint getnum_digits() const;

  friend bool operator<(const solv_cell& left, const solv_cell& right){
    return *(left.cell) < *(right.cell);
  }
  friend ostream& operator<<(ostream& os, const solv_cell& s){
    return os << *(s.cell);
  }
  friend class fp_node;

  operator int() const{
    return (int)(*cell);
  }
};

typedef set<solv_cell*> solv_set;

class solv_sudoku : public sudoku{
private:
	vector<solv_cell*>* sgrid;

	void solv_init(const uint level_bits);
	// add a NULL-check before adding to the group
	//void ginsert(set<solv_cell*>* group, solv_cell* cell) const;
public:
	// constructor
	solv_sudoku(const uint digits, const uint level_bits = LVL_ALL);
	// constructor from file
	solv_sudoku(const char* filename, const uint level_bits = LVL_ALL);
	// copy constructor
	solv_sudoku(const solv_sudoku& gs);
	// destructor
	~solv_sudoku();
	fp_node* get_thesis(const uint x, const uint y, const int thesis) const;
	solv_cell* get_cell(const uint x, const uint y) const;
	// return a set of cells in row y
	void getrow(const uint y, solv_set* group) const;
	// return a set of cells in the column x
	void getcolumn(const uint x, solv_set* group) const;
	// return a set of cells in the square n
	void getsquare(const uint n, solv_set* group) const;
	// get the [row, col, square] (dependent on group_nr) of the cell at (x,y)
	solv_set* getgroup(const uint x, const uint y, const uint group_nr) const;
	// return a set of cells in the same row as cell (x,y)
	void getrow(const uint x, const uint y, solv_set* group) const;
	// return a set of cells in the same column as cell (x,y)
	void getcolumn(const uint x, const uint y, solv_set* group) const;
	// return a set of cells in the same square as cell (x,y)
	void getsquare(const uint x, const uint y, solv_set* group) const;
	// get the n'th [row, col, square] (dependent on group_nr) 
	solv_set* getgroup(const uint n, const uint group_nr) const;
	// apply a rule to a certain cell of the sudoku, return success [validity]
	bool applyrule(const uint x, const uint y, solv_rule* r, const uint level_bits);
	// apply a rule to a suitable cell in the sudoku, return success
	bool applyrule(solv_rule* r, const uint level_bits);
	// get "neg(thesis)"
	fp_node* get_opposite(const fp_node* thesis);
};




// flood: putting a number n into a cell triggers the thesis -n in all other
// 		  cells of any of its groups and vice verca
bool flood(const uint x, const uint y, const int digit, solv_sudoku* s, const uint level_bits);
bool flood(const uint x, const uint y, solv_sudoku* s, const uint level_bits);

bool eliminate(solv_cell* cell, const int digit, const uint num_digits, const uint level_bits);
bool eliminate(const uint x, const uint y, solv_sudoku* s, const uint level_bits);

bool locate(const uint x, const uint y, const int digit, solv_sudoku* s, const uint level_bits);
bool locate(const uint x, const uint y, solv_sudoku* s, const uint level_bits);

bool group_intersect(const uint x, const uint y, const int digit, solv_sudoku* s, const uint level_bits);
bool group_intersect(const uint x, const uint y, solv_sudoku* s, const uint level_bits);

bool alignment(const uint x, const uint y, solv_sudoku* s, const uint level_bits);

bool tca(const uint x, const uint y, solv_sudoku* s, const uint level_bits, const uint restrict_level);
bool tca(const uint x, const uint y, solv_sudoku* s, const uint level_bits);


bool bigraph(const uint x, const uint y, solv_sudoku* s, const uint level_bits);
bool ebigraph(const uint x, const uint y, solv_sudoku* s, const uint level_bits);

/******** transitive chain analysis **********/
/*
# preamble: putting a digit in a certain cell means not putting
#			another digit there, but not vice versa !
#
# possible transitive chains:
#	0. not putting forces not putting
#		graph is identical to putting forces putting, with reversed edges
#	1. not putting forces putting
#		bilocation graph, ...
#	2. putting forces not putting
#		flood, ...
#	3. putting forces putting
#		bivalue, align-chain, ...
# those may be combined to form a graph of forces in which we may find
# circles and conflicting paths

# bilocal: not putting a digit in a certain cell forces putting
# 		   a certain digit [here: the same digit] in another cell 
#		   thus forcing not putting a certain digit in it
# --> transitive chains
# [~1]-->[1=~2]-->[2=~1]-->....

# bivalue: putting a digit in a certain cell forces putting
# 		   a certain digit [here: another] in another cell
# --> transitive chains

# hint1: there are more transitive chains like this:
#	aligned chains: putting a digit in a certain cell forces putting
#					a certain [here: the same] digit in another cell
# hint2: bilocation and bivalue graphs may be connected in order to
#		 get more information about the puzzle...
*/


#endif
