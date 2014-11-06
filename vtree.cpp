/***************************************************
 * vtree.cpp
 * lib to manage vitness-trees for sudoku generation
 * by M.Weller
 **************************************************/

#ifndef vtree_cpp
#define vtree_cpp

#include "vtree.h"

/************** vitness trees ***************/

void vtree_node::init(sudoku_cell* c){
	uint num_digits = c->get_num_digits();
	cell = c;
	nposs = new nposs_list(num_digits);
	// initialise the npossibility list with an empty vitness-set each
	for(uint i = 0; i < num_digits; i++)
		(*nposs)[i] = new vitness_set_t();
}

// constructor
	vtree_node::vtree_node(sudoku_cell* c){
	init(c);
}
// copy constructor
vtree_node::vtree_node(const vtree_node& vt){
	uint num_digits = cell->get_num_digits();
	cell = vt.cell;
	
	nposs = new nposs_list(num_digits);
	for(uint i = 0; i < num_digits; i++)
		for(nposs_list::iterator j = vt.nposs[i].begin(); j != vt.nposs[i].end(); j++)
			(*nposs)[i] = new vitness_set_t(**j);
}
// destructor
vtree_node::~vtree_node(){
	// vector wont delete the pointers in the vector, so we do it by hand
	for(nposs_list::iterator i = nposs->begin(); i != nposs->end(); i++) {
		for(vitness_set_t::iterator j = (*i)->begin(); j != (*i)->end(); j++)
			// delete each vitness in the current vitness-list
			delete (*j);
		// delete the possibility list entry [the vitness-list itself]
		delete (*i);
	}
	// delete the npossibility list itself
	delete nposs;
}

// return the vitness-list of a given digit of the cell
vitness_set_t* vtree_node::getvitnesslist(const uint digit) const{
	return (*nposs)[digit - 1];
}
// add a list of nodes that, together [!!!], vitness a digit
// [this is a vitness]
// may contain only one vtree_node if it alone vitesses the digit
void vtree_node::add_vitness(const uint digit, const vitness_t* vitness){
	vitness_set_t* vlist = (*nposs)[digit - 1];
	vlist->insert(new vitness_t(*vitness));
}

// remove a vitness from a digit in the npossibility list
bool vtree_node::removevitness(const uint digit, vitness_t* vitness){
	return (*nposs)[digit - 1]->erase(vitness);
}

// remove a vitness from all digits in the npossibility list
bool vtree_node::removevitness(vitness_t* clause){
	uint num_digits = cell->get_num_digits();
	bool result = false;
	for(uint i = 1; i <= num_digits; i++)
		result |= removevitness(i, clause);
	return result;
}
	
// remove all vitnesses containing a certain node from a certain digit
bool vtree_node::removenode(const uint digit, vtree_node* node){
	bool result = false;
	vitness_t::iterator j;
	vitness_set_t::iterator k;
	vitness_set_t::iterator i = (*nposs)[digit - 1]->begin();
	
	while(i != (*nposs)[digit - 1]->end()){
		// TODO: watch your iterator i when removing !!
		if((*i)->find(node) != (*i)->end()) {
			k = i; k++;
			(*nposs)[digit - 1]->erase(i);
			i = k;
			result = true;
		} else i++;
	}
	return result;
}

// remove all vitnesses containing a certain node from all digits
bool vtree_node::removenode(vtree_node* v){
	uint num_digits = cell->get_num_digits();
	bool result = false;
	for(uint i = 1; i <= num_digits; i++)
		result |= removenode(i, v);
	return result;
}

// return the number of vitnesses a certain digit has
uint vtree_node::vitnesscount(const uint digit) const{
	return (*nposs)[digit - 1]->size();
}
// return the current cell content
uint vtree_node::get_content() const{
	return cell->get_content();
}
// remove [zero] the current cell content
void vtree_node::remove_content(){
	cell->remove_content();
}
// set the cell content
void vtree_node::set_content(const uint digit){
	cell->set_content(digit);
}
uint vtree_node::get_x() const{
	return cell->get_x();
}
uint vtree_node::get_y() const{
	return cell->get_y();
}

#endif
