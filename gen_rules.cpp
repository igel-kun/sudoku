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
#ifndef gen_rules_cpp
#define gen_rules_cpp

#include "gen_rules.h"

bool gen_rule::__apply(const uint x, const uint y, gen_sudoku* s) const{
	return apply_func(x, y, s);
}
gen_rule::gen_rule(bool (*apply_f)(const uint x, const uint y, gen_sudoku*)){
	apply_func = apply_f;
}
// apply rule at a given location and return success [validity]
bool gen_rule::apply(const uint x, const uint y, gen_sudoku* s) const{
	return __apply(x,y,s);
}
// find a suitable location and apply rule there, return whether the
// rule could be applied [rule is applied just once!]
bool gen_rule::apply(gen_sudoku* s) const{
	uint digits = s->getnum_digits();
	for(uint x = 0; x < digits; x++)
		for(uint y = 0; y < digits; y++)
			if(__apply(x,y,s)) return true;
	return false;
}
gen_rule::~gen_rule(){
	apply_func = NULL;
}



gen_cell::gen_cell(sudoku_cell* _cell){
	cell = _cell;
	node = new vtree_node(cell);
}
gen_cell::gen_cell(const gen_cell& gcell){
	cell = gcell.cell;
	node = new vtree_node(*gcell.node);
}
gen_cell::~gen_cell(){
	delete node;
}
vtree_node* gen_cell::get_node() const{
	return node;
}
uint gen_cell::get_content() const{
	return cell->get_content();
}
void gen_cell::set_content(const uint digit){
	cell->set_content(digit);
}
void gen_cell::remove_content(){
	set_content(0);
}
uint gen_cell::get_x() const{
	return cell->get_x();
}
uint gen_cell::get_y() const{
	return cell->get_y();
}



void gen_sudoku::gen_init(vector<gen_cell*>* copy_grid){
	ggrid = new vector<gen_cell*>(num_digits * num_digits);
	for(uint i = 0; i < num_digits; i++)
		for(uint j = 0; j < num_digits; j++)
			if(copy_grid)
				(*ggrid)[get_index(i,j)] = new gen_cell(*((*copy_grid)[get_index(i,j)]));
			else 
				(*ggrid)[get_index(i,j)] = new gen_cell((*grid)[get_index(i,j)]);
}

// constructor
gen_sudoku::gen_sudoku(const uint digits) : sudoku(digits){
	gen_init();
	gen_init();
}
// constructor from file
gen_sudoku::gen_sudoku(const char* filename) : sudoku(filename, false){
	gen_init();
	read_from_file(filename);
}
// copy constructor
gen_sudoku::gen_sudoku(const gen_sudoku& gs) : sudoku(gs){
	gen_init(gs.ggrid);
}
// destructor
gen_sudoku::~gen_sudoku(){
	for(uint j = 0; j < num_digits * num_digits; j++)
		delete (*ggrid)[j];
	delete ggrid;
}
gen_cell* gen_sudoku::get_cell(const uint x, const uint y) const{
	return (*ggrid)[get_index(x,y)];
}
// return a set of cells in row y
void gen_sudoku::getrow(const uint y, set<gen_cell*>* group) const{
	for(uint i = 0; i < num_digits; i++)
		group->insert((*ggrid)[get_index(i,y)]);
}
// return a set of cells in the column x
void gen_sudoku::getcolumn(const uint x, set<gen_cell*>* group) const{
	for(uint i = 0; i < num_digits; i++)
		group->insert((*ggrid)[get_index(x,i)]);
}
// return a set of cells in the square n
void gen_sudoku::getsquare(const uint n, set<gen_cell*>* group) const{
	uint order = (uint)sqrt((double)num_digits);
	uint sq_x = n % order;
	uint sq_y = n / order;
	for(uint i = 0; i < order; i++)
		for(uint j = 0; j < order; j++)
			group->insert((*ggrid)[get_index(order * sq_x + i, order * sq_y + j)]);
}

// get the [row, col, square] (dependent on group_nr) of the cell at (x,y)
set<gen_cell*>* gen_sudoku::getgroup(const uint x, const uint y, const uint group_nr) const {
	set<gen_cell*>* group = new set<gen_cell*>();
	switch(group_nr){
		case 0: getrow(x,y, group); break;
		case 1: getcolumn(x,y, group); break;
		case 2: getsquare(x,y, group); break;
	}
	return group;
}

// return a set of cells in the same row as cell (x,y)
void gen_sudoku::getrow(const uint x, const uint y, set<gen_cell*>* group) const{
	getrow(y,group);
}
// return a set of cells in the same column as cell (x,y)
void gen_sudoku::getcolumn(const uint x, const uint y, set<gen_cell*>* group) const{
	getcolumn(x, group);
}
// return a set of cells in the same square as cell (x,y)
void gen_sudoku::getsquare(const uint x, const uint y, set<gen_cell*>* group) const{
	uint order = (uint)sqrt((double)num_digits);
	uint sq_x = x / order;
	uint sq_y = y / order;
	getsquare(sq_y * order + sq_x, group);
}
// get the n'th [row, col, square] (dependent on group_nr) 
set<gen_cell*>* gen_sudoku::getgroup(const uint n, const uint group_nr) const {
	set<gen_cell*>* group = new set<gen_cell*>();
	switch(group_nr){
		case 0: getrow(n, group); break;
		case 1: getcolumn(n, group); break;
		case 2: getsquare(n, group); break;
	}
	return group;
}
// apply a rule to a certain cell of the sudoku, return success [validity]
bool gen_sudoku::applyrule(const uint x, const uint y, const gen_rule* r){
	return r->apply(x, y, this);
}

// apply a rule to a suitable cell in the sudoku, return success
bool gen_sudoku::applyrule(const gen_rule* r){
	return r->apply(this);
}
// remove the content of a specific node and remove it from all vitness
// lists. return if it was removed from any vitness lists
bool gen_sudoku::remove_node_content(vtree_node* node){
	bool result = false;
	for(uint i = 0; i < num_digits; i++)
		for(uint j = 0; j < num_digits; j++)
			result |= ((*ggrid)[get_index(i, j)])->get_node()->removenode(node);
	node->remove_content();
	return result;
}

// add the vitness for digit to all nodes in the group
void gen_sudoku::add_vitness(const vitness_t* v, const uint digit, set<gen_cell*>* group){
	for(set<gen_cell*>::iterator i = group->begin(); i != group->end(); i++)
		(*i)->get_node()->add_vitness(digit, v);
}


// definition needs to go after class sudoku, bcause of getnum_digits()
// reverse_flood: add a number as vitness for all fields in it's groups
bool reverse_flood(const uint x, const uint y, gen_sudoku* s){
	set<gen_cell*>* group;

	gen_cell* cell = s->get_cell(x,y);
	// cannot apply to empty cell
	if(!cell->get_content()) return false; 

	vtree_node* node = cell->get_node();
	// create a vitness with the current node as single entry
	vitness_t* vitness = new vitness_t();
	vitness->insert(node);
	
	// flood all 3 groups
	for(uint j = 0; j < 3; j++){
		group = s->getgroup(x,y,j);
		
		s->add_vitness(vitness, cell->get_content(), group);
		delete group;
	}
	delete vitness;
	return true;
}

// reverse eliminate: tries to undo D.Eppsteins eliminate.
// 
// a cell-content can be removed if the deletion of all other
// digits are vitnessed by some nodes
// [and it vitnesses itself just once [otherwise the puzzle is
// invalid]] (TODO: write a proper detection for this case)
bool reverse_eliminate(const uint x, const uint y, gen_sudoku* s){
	uint digits = s->getnum_digits();
	gen_cell* cell = s->get_cell(x,y);
	bool applicable = true;


	// cannot apply to empty cell
	if(!cell->get_content()) return false;

	vtree_node* node = cell->get_node();

	// create a vitness with the current node as single entry
	vitness_t* vitness = new vitness_t();
	vitness->insert(node);
	
	vitness_set_t* vitnesslist = node->getvitnesslist(node->get_content());
	// rule can be applied if all digits of the cell have a vitness and the only
	// vitness for it's content is itself
	for(uint i = 1; i <= digits; i++)
		applicable &= (node->vitnesscount(i) > 0);
	applicable &= (vitnesslist->size() == 1);
	applicable &= (**(vitnesslist->begin()) == *vitness);
	
	dbgprint("reverse-eliminate is %sapplicable\n",applicable?"":"not ");
	delete vitness;
	
	// apply the rule if it is applicable
	if(!applicable) return false; else {
		dbgprint("applying reverse-eliminate to (%d, %d)\n", x, y);
		s->remove_node_content(node);
		return true;
	}
}

// reverse locate: tries to undo D.Eppsteins locate rule
// 
// a cell-content can be removed if, in one of its groups, all cells
// without content [!get_content()] have at least 2 vitnesses for the
// digit equaling the cell-content which is to be removed, namely
// the cell itself [it still is in the same group] and any other
bool reverse_locate(const uint x, const uint y, gen_sudoku* s){
	bool applicable;
	set<gen_cell*>* group;

	uint digit = s->get_cell(x,y)->get_content();

	// cannot apply to empty cell
	if(!digit) return false;

	for(uint i = 0; i < 3; i++){
		group = s->getgroup(x,y,i);

		// rule can be applied if all the nodes in the current group have their
		// 'digit' vitnessed at least twice, except if they have content
		applicable = true;
		for(set<gen_cell*>::iterator i = group->begin(); i != group->end(); i++)
			if(((*i)->get_node()->vitnesscount(digit) < 2) && !((*i)->get_content()))
				applicable=false;
		
		delete group;
		dbgprint("reverse-locate is %sapplicable\n",applicable?"":"not ");

		// if the rule is applicable, there is no need to check further groups
		if(applicable) break;
	}
	
	// apply the rule if it is applicable
	if(!applicable) return false; else {
		dbgprint("applying reverse-locate to (%d, %d)\n", x, y);
		s->get_cell(x,y)->remove_content();
		return true;
	}
}

#endif

