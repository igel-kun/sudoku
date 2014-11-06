/***************************************************
 * vtree.h
 * lib to manage vitness-trees for sudoku generation
 * by M.Weller
 **************************************************/

#ifndef vtree_h
#define vtree_h

#include <set> // STL lists for vtree_list
#include <vector>

#include "sudoku.h"

using namespace std;


/************** vitness trees ***************/

class vtree_node;

typedef set<vtree_node*> vitness_t;		// a vitness is a set of vtree_nodes
										// which, together, vitness the deletion
										// of some digit in a cell
class vitness_t_cmp{
public:
	bool operator()(const vitness_t* left, const vitness_t* right){
		return *left < *right;
	}
};

typedef set<vitness_t*, vitness_t_cmp> vitness_set_t;		
											// a set of vitnesses
											// which, each alone, vitness the deletion
											// of some digit in a cell
											// [for each vitness, vitnessing the deletion
											// means that all it's vtree_nodes must
											// vitness the deletion]

typedef vector<vitness_set_t*> nposs_list;	
											// each digit has a set of vitnesss
											// a possibility list is a list of
											// lists of vitnesses 
											// [one vitness-set for each digit]


// vitness-tree node, basically a cell in a sudoku grid
// TODO: write copy-constructor
class vtree_node {		
private:
	nposs_list* nposs;	// list of non-possibilities and their 
						// vitnesses [a matrix each] for a digit to be
						// possible at this cell, poss[digit] needs
						// to equal the empty set
						// MIND THE OFFSET DUE TO USING nposs[0] for 1
	sudoku_cell* cell;

	void init(sudoku_cell* c);
public:
	// constructor
	vtree_node(sudoku_cell* c);
	// copy constructor
	vtree_node(const vtree_node& vt);
	// destructor
	~vtree_node();
	// return the vitness-list of a given digit of the cell
	vitness_set_t* getvitnesslist(const uint digit) const;
	// add a list of nodes that, together [!!!], vitness a digit
	// [this is a vitness]
	// may contain only one vtree_node if it alone vitesses the digit
	void add_vitness(const uint digit, const vitness_t* vitness);
	// remove a vitness from a digit in the npossibility list
	bool removevitness(const uint digit, vitness_t* vitness);
	// remove a vitness from all digits in the npossibility list
	bool removevitness(vitness_t* clause);
	// remove all vitnesses containing a certain node from a certain digit
	bool removenode(const uint digit, vtree_node* node);
	// remove all vitnesses containing a certain node from all digits
	bool removenode(vtree_node* v);
	// return the number of vitnesses a certain digit has
	uint vitnesscount(const uint digit) const;
	// return the current cell content
	uint get_content() const;
	// remove [zero] the current cell content
	void remove_content();
	// set the cell content
	void set_content(const uint digit);
	uint get_x() const;
	uint get_y() const;
};

#endif
