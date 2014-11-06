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

#ifndef solv_rules_cpp
#define solv_rules_cpp

#include "solv_rules.h"
#include <algorithm> // for set_difference
#include <unordered_set>
#include "align.h"

bool solv_rule::__apply(const uint x, const uint y, solv_sudoku* s, const uint level_bits) const{
	return apply_func(x, y, s, level_bits);
}
solv_rule::solv_rule(bool (*apply_f)(const uint x, const uint y, solv_sudoku*, const uint)){
	apply_func = apply_f;
	lastx = 0;
	lasty = 0;
}
// apply rule at a given location and return success [validity]
bool  solv_rule::apply(const uint x, const uint y, solv_sudoku* s, const uint level_bits) const{
	return __apply(x,y,s,level_bits);
}

// find a suitable location and apply rule there, return whether the
// rule could be applied [rule is applied just once!]
bool solv_rule::apply(solv_sudoku* s, const uint level_bits){
	const uint digits = s->getnum_digits();

	uint stopx = lastx;
	uint stopy = lasty;
	do{
		if(__apply(lastx,lasty,s, level_bits)) return true;
		lasty++;
		if(lasty == digits){
			lasty = 0;
			lastx = (lastx + 1) % digits;
		}
	} while(!((lastx == stopx) && (lasty == stopy)));
	return false;
}
solv_rule::~solv_rule(){
	apply_func = NULL;
}



void solv_cell::sinit(const uint _num_digits){
	int content = cell->get_content();
  num_digits = _num_digits;
	nodes = new vector<fp_node*>(2 * _num_digits + 1);
	// if the connected cell already has a content [!=0], then there is only 1 positive thesis
	// all negative theses except -content are triggered as well
	if(content){
		for(int i = 1; i < (int)_num_digits; i++) if(i != content)
			(*nodes)[i + _num_digits] = NULL;
		for(int i = -_num_digits; i < 0; i++) if(i != -content){
			(*nodes)[i + _num_digits] = new fp_node(this, i);
			(*nodes)[i + _num_digits]->set_trigger(0);
		}
		(*nodes)[content + _num_digits] = new fp_node(this, content);
		(*nodes)[content + _num_digits]->set_trigger(0);
	} else {
		for(int i = -_num_digits; i <= (int)_num_digits; i++) 
			if(i)
				(*nodes)[i + _num_digits] = new fp_node(this, i);
			else
				(*nodes)[i + _num_digits] = NULL;
	}
}

solv_cell::solv_cell(const uint _num_digits, const uint _x, const uint _y, const uint _content){
	cell = new sudoku_cell(_num_digits, _x, _y, _content);
	sinit(_num_digits);
}

solv_cell::solv_cell(sudoku_cell* _cell){
	cell = _cell;
	sinit(cell->get_num_digits());
}

solv_cell::solv_cell(const solv_cell& _scell){
	cell = _scell.cell;
  num_digits = _scell.num_digits;
	nodes = new vector<fp_node*>(*_scell.nodes);
}

solv_cell::~solv_cell(){
	for(uint i = 0; i < nodes->size(); i++)
		if((*nodes)[i]) {
      delete (*nodes)[i];
      (*nodes)[i] = NULL;
    }
	delete nodes;
}

// return the fp_node of the given number (digit) for this cell
// somthing special: indices go from -num_digits to num_digits
// however: 0 is invalid
fp_node*& solv_cell::operator[](const int digit) {
  const uint digits(cell->get_num_digits());

  if((-digit > (int)digits) || (digit > (int)digits) || (digit == 0))
    diewith("invalid access to nonexistent digit " << digit << endl);

	return (*nodes)[digit + cell->get_num_digits()];
}

fp_node* const& solv_cell::operator[](const int digit) const{
  const uint digits(cell->get_num_digits());

  if((-digit > (int)digits) || (digit > (int)digits) || (digit == 0))
    diewith("invalid access to nonexistent digit " << digit << endl);

	return (*nodes)[digit + cell->get_num_digits()];
}


uint solv_cell::get_content() const{
	return cell->get_content();
}
bool solv_cell::remove_thesis(const int digit, const uint level_bits){
	const uint _num_digits = cell->get_num_digits();
  fp_node*& node = (*nodes)[digit + _num_digits];

	
	// cannot remove, what was removed already
	if(!node) {
    dbgout << "thesis " << digit << " doesn't exist for " << *this << endl;
    return false;
  } else 
    dbgout << "removing thesis " << *node << endl;

	// removing a triggered thesis is very wrong...
	if(node->is_triggered())
    diewith("sudoku is invalid: trying to remove triggered thesis " << *node << endl);
	
	delete node;
  node = NULL;

  // this function does not set triggers, it is called by set_trigger only!!!
	// if x can never be triggered, then -x must be triggered and vice versa
	//(*nodes)[-digit + _num_digits]->set_trigger(level_bits);
	return true;
}


void solv_cell::set_content(const uint digit, const uint level_bits){
	dbgprint("setting content for [%d,%d] to %d\n",cell->get_x(), cell->get_y(), digit);
	if(digit <= 0)
    diewith("removing content from a solv_cell???"<<endl);

	cell->set_content(digit);
	// trigger the thesis if it is not already
	(*nodes)[digit + cell->get_num_digits()]->set_trigger(level_bits);
}
void solv_cell::remove_content(){
	set_content(0,0);
}
uint solv_cell::get_x() const{
	return cell->get_x();
}
uint solv_cell::get_y() const{
	return cell->get_y();
}
uint solv_cell::count_poss() const{
	uint res = 0;
	const uint digits = cell->get_num_digits();
	for(uint i = 1; i <= digits; i++)
		if((*nodes)[i + digits]) res++;
	return res;
}
uint solv_cell::getnum_digits() const{
  return num_digits;
}


void solv_sudoku::solv_init(const uint level_bits){
	// init the sgrid
	sgrid = new vector<solv_cell*>(num_digits * num_digits);
	for(uint i = 0; i < num_digits; i++)
		for(uint j = 0; j < num_digits; j++)
			(*sgrid)[get_index(i,j)] = new solv_cell((*grid)[get_index(i,j)]);
}

// constructor
solv_sudoku::solv_sudoku(const uint digits, const uint level_bits) : sudoku(digits){
	solv_init(level_bits);
}
// constructor from file
solv_sudoku::solv_sudoku(const char* filename, const uint level_bits) : sudoku(){
	char* s = read_first_line(filename);

	init(strlen(s));
	dbgprint("number of digits: %d, input is %s, first line is %s\n", num_digits, strcmp(filename, "stdin")?"a file":"stdin", s);
	solv_init(level_bits);
	
	read_from_file(filename, strcmp(filename,"stdin")?NULL:s);
	free(s);

	uint c;
	for(uint x = 0; x < num_digits; x++)
		for(uint y = 0; y < num_digits; y++)
			if((c = (*grid)[get_index(x,y)]->get_content()))
				(*sgrid)[get_index(x,y)]->set_content(c, level_bits);

}
// copy constructor [TODO]
solv_sudoku::solv_sudoku(const solv_sudoku& gs) : sudoku(gs){
	printf("oh noes, copy construction...\n");
	exit(0);
}
// destructor
solv_sudoku::~solv_sudoku(){
	for(uint i = 0; i < num_digits * num_digits; i++){
		delete (*sgrid)[i];
    (*grid)[i] = NULL;
  }
	delete sgrid;
  sgrid = NULL;
}
fp_node* solv_sudoku::get_thesis(const uint x, const uint y, const int thesis) const{
	return (*(*sgrid)[get_index(x,y)])[thesis];
}
solv_cell* solv_sudoku::get_cell(const uint x, const uint y) const{
	return (*sgrid)[get_index(x,y)];
}

// return a set of cells in row y
void solv_sudoku::getrow(const uint y, set<solv_cell*>* group) const{
	for(uint i = 0; i < num_digits; ++i)
		group->insert((*sgrid)[get_index(i,y)]);
}
// return a set of cells in the column x
void solv_sudoku::getcolumn(const uint x, set<solv_cell*>* group) const{
	for(uint i = 0; i < num_digits; i++)
		group->insert((*sgrid)[get_index(x,i)]);
}

// return a set of cells in the square n
void solv_sudoku::getsquare(const uint n, set<solv_cell*>* group) const{
	const uint order = (uint)sqrt((double)num_digits);
	const uint sq_x = n % order;
	const uint sq_y = n / order;
	for(uint i = 0; i < order; i++)
		for(uint j = 0; j < order; j++)
			group->insert((*sgrid)[get_index(order * sq_x + i, order * sq_y + j)]);
}

// get the [row, col, square] (dependent on group_nr) of the cell at (x,y)
set<solv_cell*>* solv_sudoku::getgroup(const uint x, const uint y, const uint group_nr) const {
	set<solv_cell*>* group = new set<solv_cell*>();
	switch(group_nr){
		case 0: getrow(x,y, group); break;
		case 1: getcolumn(x,y, group); break;
		case 2: getsquare(x,y, group); break;
	}
	return group;
}


// return a set of cells in the same row as cell (x,y)
void solv_sudoku::getrow(const uint x, const uint y, set<solv_cell*>* group) const{
	getrow(y,group);
}
// return a set of cells in the same column as cell (x,y)
void solv_sudoku::getcolumn(const uint x, const uint y, set<solv_cell*>* group) const{
	getcolumn(x, group);
}
// return a set of cells in the same square as cell (x,y)
void solv_sudoku::getsquare(const uint x, const uint y, set<solv_cell*>* group) const{
	const uint order = (uint)sqrt((double)num_digits);
	const uint sq_x = x / order;
	const uint sq_y = y / order;
	getsquare(sq_y * order + sq_x, group);
}

// get the n'th [row, col, square] (dependent on group_nr) 
set<solv_cell*>* solv_sudoku::getgroup(const uint n, const uint group_nr) const {
	set<solv_cell*>* group = new set<solv_cell*>();
	switch(group_nr){
		case 0: getrow(n, group); break;
		case 1: getcolumn(n, group); break;
		case 2: getsquare(n, group); break;
	}
	return group;
}

// apply a rule to a certain cell of the sudoku, return success [validity]
bool solv_sudoku::applyrule(const uint x, const uint y, solv_rule* r, const uint level_bits){
	return r->apply(x, y, this, level_bits);
}

// apply a rule to a suitable cell in the sudoku, return success
bool solv_sudoku::applyrule(solv_rule* r, const uint level_bits){
	return r->apply(this, level_bits);
}
	

// get "neg(thesis)"
fp_node* solv_sudoku::get_opposite(const fp_node* thesis){
	solv_cell* sc = (*sgrid)[get_index(thesis->get_cell()->get_x(), thesis->get_cell()->get_y())];
	return (*sc)[-(thesis->get_thesis())];
}
	


/******************************* flood *******************************/

// group_flood: putting a number +n into a cell triggers the thesis -n in all other
// 		  cells of any of its groups
bool group_flood(const uint x, const uint y, const int digit, solv_sudoku* s, const uint level_bits){
	// cannot apply to negative thesis
	if(digit <= 0) return false;
	if(!s) diewith("flood got invalid sudoku"<<endl);


  fp_node* thesis = s->get_thesis(x,y,digit);
  // cannot apply to falsified thesis
  if(!thesis) return false;

	bool result =  false;
	for(uint i = 0; i < 3; ++i){
	  set<solv_cell*>* group = s->getgroup(x,y,i);
    
    dbgout << "erasing " << *(thesis->get_cell()) << endl;
		group->erase(thesis->get_cell());

    // for each thesis -digit in the group, add a trigger from "thesis" to it
    for(set<solv_cell*>::iterator i = group->begin(); i != group->end(); ++i){
      fp_node* node = (**i)[-digit];
      
      if(node){
        // if (**i)[-digit] can be triggered
        if(node->add_trigger(thesis, LVL_FLOOD, level_bits))
          result = true;
      } else {
        // if (**i)[-digit] cannot be triggered, then thesis cannot be triggered
        fp_node* neg_thesis = s->get_opposite(thesis);
        neg_thesis->set_trigger(level_bits);
        // of course, thesis is now invalid
        return result;
      }
    }
	}
  if(result) dbgout << "ha, i was able to add flood rule" << endl;
	return result;
}

// cell_flood: putting a number +n into a cell triggers the thesis -m for all m != n
//      in this cell
bool cell_flood(const uint x, const uint y, const int digit, solv_sudoku* s, const uint level_bits){
	// cannot apply to negative thesis
	if(digit <= 0) return false;
	if(!s) diewith("flood got invalid sudoku"<<endl);

  fp_node* thesis = s->get_thesis(x,y,digit);
  // cannot apply to falsified thesis
  if(!thesis) return false;
  
  dbgout << "cell-flooding at (" << x << "," << y << ")" << endl;

  bool result = false;
  solv_cell& cell = *(thesis->get_cell());
  for(int i = s->getnum_digits(); i != 0; --i) if(i != digit){
    dbgout << "digit " << i << endl;
    if(cell[-i]){
      dbgout << "cell[" << -i << "] exists at " << cell[-i] << ":" << endl;
      dbgout << *(cell[-i]) << endl;
      result |= ( cell[-i]->add_trigger(thesis, LVL_FLOOD, level_bits) != NULL);
    } else {
      // if cell[-i] doesn't exist, then cell[digit] cannot be triggered.
      // Hence, trigger cell[-digit]
      dbgout << "cell[" << -i << "] doesn't exist, triggering " << *(cell[-digit]) << endl;
      cell[-digit]->set_trigger(level_bits);
    
      // note that this invalidates *thesis
      return true;
    }
  }

	return result;
}


bool flood(const uint x, const uint y, solv_sudoku* s, const uint level_bits){
	dbgprint("flooding (%d,%d)\n=========================\n", x, y);
	uint digit = s->getnum_digits();
	bool result = false;
  while(digit){
		result |= group_flood(x,y, digit,s, level_bits);
    result |= cell_flood(x, y, digit, s, level_bits);
    --digit;
  }
	dbgprint("done flooding (%d,%d)\n=========================\n", x, y);
	return result;
}



/************************* eliminate *************************/
// eliminate: mimics D.Eppsteins eliminate.
// 
// if a cell has only one possible digit left, fill it in
bool eliminate(solv_cell* cell, const int digit, const uint num_digits, const uint level_bits){
	if(digit <= 0) return false;
	if(!cell) diewith("got invalid sudoku cell in eliminate"<<endl);

	fp_node* node = (*cell)[digit];
	if(node) {
	  fp_node_set* neg_theses = new fp_node_set();
		for(int j = -num_digits; j < 0; ++j) if(j != -digit){
      fp_node* tmp_node = (*cell)[j];
      if(tmp_node)
        if(! tmp_node->is_triggered())
          neg_theses->insert(tmp_node);
    }
    dbgout << "calling add_trigger with " << *neg_theses << " --> " << *node << endl;
		if(node->add_trigger(neg_theses, LVL_ELIMINATE, level_bits)){
      return true;
    }else{
      delete neg_theses;
      return false;
    }
	}
	return false;
}
bool eliminate(const uint x, const uint y, solv_sudoku* s, const uint level_bits){
	dbgprint("eliminate (%d,%d)\n========================================\n", x, y);
  const uint num_digits = s->getnum_digits();
	uint digit = num_digits;
	solv_cell* cell = s->get_cell(x,y);
	bool result = false;
  while(digit){
    dbgout << "eliminating for " << *cell << "=" << digit << " (current res: " << result << ")" << endl;

		result |= eliminate(cell, digit--, num_digits, level_bits);
  }
	return result;
}


/*************************** locate ******************************/
// locate: tries to mimic D.Eppsteins locate rule
// if a digit has only one possibility for any group left, fill it in
bool locate(const uint x, const uint y, const int digit, solv_sudoku* s, const uint level_bits){
	dbgout << "(" << x << "," << y << ")" << endl << "===========================" << endl;
	set<solv_cell*>* group;
	fp_node* thesis;

	// cannot apply to negative thesis
	if(digit <= 0) return false;
		
	bool result = true;
	for(uint i = 0; i < 3; i++){
		thesis = s->get_thesis(x,y,digit);
		if(!thesis) return false;

		group = s->getgroup(x,y,i);
		group->erase(thesis->get_cell());

		if(!thesis->add_triggers(group, -digit, LVL_LOCATE, level_bits)){
      result = false;
      delete group;
    }
	}
	return result;
}
bool locate(const uint x, const uint y, solv_sudoku* s, const uint level_bits){
	uint digits = s->getnum_digits();
	bool result = false;
  while(digits)
		result |= locate(x,y, digits--,s, level_bits);
	return result;
}
/******** align **********/

/******** subproblem **********/

/******** digit **********/

/******** group intersection ********/
/* group intersection: all possibilities of a digit in some group A are also in
 * _one_ other group B, then remove all possibilities of this digit from B\A
 */
// if all possibilities of digit in groupA are also in groupB, then remove all
// possiblities of digit in groupB that are not in groupA
bool group_intersect(const set<solv_cell*>* groupA, const set<solv_cell*>* groupB, const int digit, solv_sudoku* s, const uint level_bits){

  fp_node_set trigger_set;
  set<solv_cell*> AminusB;
  std::set_difference(groupA->begin(), groupA->end(), groupB->begin(), groupB->end(),
        std::inserter(AminusB, AminusB.end()));

  set<solv_cell*> BminusA;
  std::set_difference(groupB->begin(), groupB->end(), groupA->begin(), groupA->end(),
        std::inserter(BminusA, BminusA.end()));
 
  // triggered by all cells in groupA - groupB containing -digit
  for(set<solv_cell*>::iterator i = AminusB.begin(); i != AminusB.end(); ++i){
    fp_node* node = (**i)[-digit];
    if(!node){
      dbgout << "impossible to trigger " << **i << "[" << -digit << "]!" << endl;
      return false;
    } else {
    dbgout << "adding " << *node << " to the trigger set" << endl;
      trigger_set.insert(node);
    }
  }

  dbgout << "constructed trigger set " << trigger_set << endl;

  // impact: noone in groupB - groupA can have digit
  bool result = false;
  for(set<solv_cell*>::const_iterator i = BminusA.begin(); i != BminusA.end(); ++i){
    dbgout << "digit: " << digit << " result so far: " << result << endl;
    fp_node_set* tmp_set = new fp_node_set(trigger_set);

    fp_node* node = (**i)[-digit];
    if(node){
      const bool tmp_result = node->add_trigger(tmp_set, LVL_GROUP, level_bits);
      if(!tmp_result) delete tmp_set;
      result |= tmp_result;
    }
  }

  return result;
}

bool group_intersect(const uint x, const uint y, const int digit, solv_sudoku* s, const uint level_bits){
  if(!s) exit(1);
  if(digit < 0) return false;

  set<solv_cell*> groupA, groupB;
 
  // the rule makes only sence if one of the groups is a square
  s->getsquare(x, y, &groupA);
  s->getrow(x, y, &groupB);
  
  bool result = false;

  result |= group_intersect(&groupA, &groupB, digit, s, level_bits);
  result |= group_intersect(&groupB, &groupA, digit, s, level_bits);

  groupB.clear();
  s->getcolumn(x, y, &groupB);

  result |= group_intersect(&groupA, &groupB, digit, s, level_bits);
  result |= group_intersect(&groupB, &groupA, digit, s, level_bits);

  return result;
}


bool group_intersect(const uint x, const uint y, solv_sudoku* s, const uint level_bits){
	dbgprint("group intersect (%d,%d)\n========================================\n", x, y);
	uint digit = s->getnum_digits();
	bool result = false;
  while(digit)
		result |= group_intersect(x, y, digit--, s, level_bits);
	return result;
}


bool alignment(const uint x, const uint y, solv_sudoku* s, const uint level_bits){
	dbgprint("alignment (%d,%d)\n========================================\n", x, y);
  bool result = false;
  uint num_digits = s->getnum_digits();
  for(uint group_nr = 0; group_nr < 3; ++group_nr){
    dbgout << "aligning group "<< group_nr << " at " << x << "," << y << endl;
    solv_set* group = s->getgroup(x, y, group_nr);
    
    dbgout << "aligning cells" << endl;
    for(solv_set::iterator i = group->begin(); i != group->end(); ++i)
      result |= cell_align(group, *i, level_bits);

    dbgout << "aligning digits" << endl;
    for(uint i = num_digits; i != 0; --i)
      result |= digit_align(group, i, level_bits);
  }
  return result;
}



/******** bigraph **********/
/* bigraph: tries to mimic D.Eppsteins bivalue and bilocation rules
 * same as tca, just with the restriction that locate and eliminate
 * chains may only be followed if they have only one trigger
 * this does in fact combine bilocation and bivalue graphs
*/

bool bigraph(const uint x, const uint y, solv_sudoku* s, const uint level_bits){
	return tca(x,y,s,level_bits, 1);
}

bool ebigraph(const uint x, const uint y, solv_sudoku* s, const uint level_bits){
	return tca(x,y,s,level_bits, 2);
}
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

// find conflicting circles in the fp-graph
bool tca(const uint x, const uint y, solv_sudoku* s, const uint level_bits, const uint restrict_level){
	solv_cell* sc = s->get_cell(x,y);
	// cannot apply to fixed cell
	if(sc->get_content()) return false;

	const uint digits = s->getnum_digits();
	bool result = false;

	for(int i = -digits; i <= (int)digits; i++) if(i) if((*sc)[i]) if(!(*sc)[i]->is_triggered()){
    dbgout << *sc << ": trying to reach " << -i << " from " << i << endl;
		if(fp_gap((*sc)[i], (*sc)[-i], level_bits, restrict_level)) {
			dbgout << "success: triggering " << *((*sc)[-i]) << endl;
			result = true;
      (*sc)[-i]->set_trigger(level_bits);
		} else dbgout << "done" << endl;
	}

	return result;
}

bool tca(const uint x, const uint y, solv_sudoku* s, const uint level_bits){
	return tca(x,y,s,level_bits,false);
}
#endif
