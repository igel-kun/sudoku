/*********************************************
 * Ruleset for sudoku-generation
 * Rules by D.Eppstein
 * Code by M.Weller
 *********************************************

 Rules are:
 	reverse-flood
	reverse-eliminate
	reverse-locate


 Rules return true iff they could be applied to the given cell
*/
#ifndef gen_rules_h
#define gen_rules_h

#include "vtree.h"
#include "sudoku.h"

class gen_sudoku;

class gen_rule {
private:
	bool (*apply_func)(const uint x, const uint y, gen_sudoku*);

	bool __apply(const uint x, const uint y, gen_sudoku* s) const;
public:
	gen_rule(bool (*apply_f)(const uint x, const uint y, gen_sudoku*));
	// apply rule at a given location and return success [validity]
	bool apply(const uint x, const uint y, gen_sudoku* s) const;
	// find a suitable location and apply rule there, return whether the
	// rule could be applied [rule is applied just once!]
	bool apply(gen_sudoku* s) const;
	~gen_rule();
};

class gen_cell {
protected:
	vtree_node* node;
	sudoku_cell* cell;
public:
	gen_cell(sudoku_cell* _cell);
	gen_cell(const gen_cell& gcell);
	~gen_cell();
	vtree_node* get_node() const;
	uint get_content() const;
	void set_content(const uint digit);
	void remove_content();
	uint get_x() const;
	uint get_y() const;
};



class gen_sudoku : public sudoku{
private:
	vector<gen_cell*>* ggrid;

	void gen_init(vector<gen_cell*>* copy_grid = NULL);

public:
	// constructor
	gen_sudoku(const uint digits);
	// constructor from file
	gen_sudoku(const char* filename);
	// copy constructor
	gen_sudoku(const gen_sudoku& gs);
	// destructor
	~gen_sudoku();
	gen_cell* get_cell(const uint x, const uint y) const;
	// return a set of cells in row y
	void getrow(const uint y, set<gen_cell*>* group) const;
	// return a set of cells in the column x
	void getcolumn(const uint x, set<gen_cell*>* group) const;
	// return a set of cells in the square n
	void getsquare(const uint n, set<gen_cell*>* group) const;
	// get the [row, col, square] (dependent on group_nr) of the cell at (x,y)
	set<gen_cell*>* getgroup(const uint x, const uint y, const uint group_nr) const;
	// return a set of cells in the same row as cell (x,y)
	void getrow(const uint x, const uint y, set<gen_cell*>* group) const;
	// return a set of cells in the same column as cell (x,y)
	void getcolumn(const uint x, const uint y, set<gen_cell*>* group) const;
	// return a set of cells in the same square as cell (x,y)
	void getsquare(const uint x, const uint y, set<gen_cell*>* group) const;
	// get the n'th [row, col, square] (dependent on group_nr) 
	set<gen_cell*>* getgroup(const uint n, const uint group_nr) const;
	// apply a rule to a certain cell of the sudoku, return success [validity]
	bool applyrule(const uint x, const uint y, const gen_rule* r);
	// apply a rule to a suitable cell in the sudoku, return success
	bool applyrule(const gen_rule* r);
	// remove the content of a specific node and remove it from all vitness
	// lists. return if it was removed from any vitness lists
	bool remove_node_content(vtree_node* node);
	// add the vitness for digit to all nodes in the group
	void add_vitness(const vitness_t* v, const uint digit, set<gen_cell*>* group);
};

// reverse_flood: add a number as vitness for all fields in it's groups
bool reverse_flood(const uint x, const uint y, gen_sudoku* s);
// reverse eliminate: tries to undo D.Eppsteins eliminate.
// 
// a cell-content can be removed if the deletion of all other
// digits are vitnessed by some nodes
// [and it vitnesses itself just once [otherwise the puzzle is
// invalid]] (TODO: write a proper detection for this case)
bool reverse_eliminate(const uint x, const uint y, gen_sudoku* s);
// reverse locate: tries to undo D.Eppsteins locate rule
// 
// a cell-content can be removed if, in one of its groups, all cells
// without content [!get_content()] have at least 2 vitnesses for the
// digit equaling the cell-content which is to be removed, namely
// the cell itself [it still is in the same group] and any other
bool reverse_locate(const uint x, const uint y, gen_sudoku* s);

#endif

