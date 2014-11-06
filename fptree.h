/***************************************************
 * fptree.h
 * lib to manage force-propagation-trees for sudoku solution
 * by M.Weller
 **************************************************
 *
 *
 *  ___________        ____________________________          ____________________________
 * | solv_cell |  +-->|           fp_node          |        |           fp_node          |
 * |-----------|  |   |----------------------------|        |----------------------------|
 * |nodes:     |  |   |triggers:___     __impacts:_|        |triggers:___     __impacts:_|
 * |-9:fp_node:|--+   | fp_trigger |   |fp_trigger:|------->| fp_trigger |   |fp_trigger:|----...
 * |-8:fp_node:|      |------------|   |-----------|        |------------|   |-----------|
 * |...        |      | fp_trigger |   |fp_trigger:|        | fp_trigger |   |fp_trigger:|----...
 * | 9:fp_node:|      |------------|   |-----------|        |------------|   |-----------|
 * |___________|  +-->| fp_trigger |   |fp_trigger:|--+     | fp_trigger |   |fp_trigger:|----...
 *                |   |____________|___|___________|  |     |____________|___|___________|
 *                |                                   |      
 *               ...                                  |      ____________________________
 *                                                    |     |         fp_node            |
 *                                                    |     |----------------------------|
 *                                                    |     |triggers:___     __impacts:_|
 *                                                    +---->| fp_trigger |   |fp_trigger:|
 *                                                          |------------|   |-----------|
 *                                                          | fp_trigger |   |fp_trigger:|
 *                                                          |____________|___|___________|
 */




#ifndef fptree_h
#define fptree_h

#include <set> // STL lists for vtree_list
#include <list>
#include <unordered_set> // for trigger sets
#include <iostream>

#include "sudoku.h"

using namespace std;

class solv_cell;
/***************** force propagation trees **********************/

class fp_node;

typedef set<fp_node*> fp_node_set;		// an impact is a set of 
											// treenodes which are
											// affected by applying
											// a certain rule to a 
											// hypothesis

typedef set<fp_node_set*> fp_node_table;	// a set of impacts, each for a
											// different rule applied to
											// certain hypotheses



class fp_trigger{
  public:
  	fp_node_set* node_set;
  	fp_node* owner;
  	uint level;
  
  fp_trigger(fp_node_set* _node_set, fp_node* _owner, const uint _level):
    node_set(_node_set), owner(_owner), level(_level) {};

  operator int() const;

  friend ostream& operator<<(ostream& os, const fp_trigger& t);

};


struct trigger_hash{
  size_t operator()(const fp_trigger* tr) const{
    return (int)(*tr);
  }
};
struct trigger_equal{
  bool operator()(const fp_trigger* left, const fp_trigger* right) const{
    const bool result=((int)(*left) == (int)(*right));
    return result;
  }
};


bool trigger_triggered(const fp_trigger* trigger, const uint level_bits);

typedef const fp_trigger* fp_trigger_p;
typedef const fp_trigger* fp_impact_p;
typedef unordered_set<fp_trigger_p, trigger_hash, trigger_equal> fp_trigger_set;
typedef set<fp_trigger_p> fp_impact_set;

// force propagation tree node: a cell and one of its 
// possibilities/non-possibilities in a sudoku grid. 
// basically a hypothesis of placing/not placing a digit there
class fp_node {		
private:
	solv_cell* cell;
	const int thesis;	// -n means "not placing digit n"
				// n means "placing digit n"
	bool triggered;	// this thesis must be true for some reason
					// [f.ex. because the opposite can not happen]
	fp_trigger_set* triggers;	// set of sets of nodes, any set of nodes
								// in 'triggers' may trigger the thesis
	fp_impact_set* impacts;	// set of sets of nodes which are triggered by
							// this thesis

	void init(const fp_trigger_set* _triggers, const fp_impact_set* _impacts);
public:
	// constructor
	fp_node(solv_cell* _cell, const int _thesis);
	// copy constructor
	fp_node(const fp_node& fpnode);
	// destructor
	~fp_node();
	int get_thesis() const;
	bool is_triggered() const;
	solv_cell* get_cell() const;
	void set_trigger(const uint level_bits);
	// add a trigger to the trigger set and return it
	// return a pointer to the trigger who contains
	// the given fp_node_set
	fp_trigger_p add_trigger(fp_node_set* _tr_node_set, const uint level, const uint level_bits);
	// add a single fp_node to the trigger set
	fp_trigger_p add_trigger(fp_node* _tr_noder, const uint level, const uint level_bits);
  // convinience function for adding a triggers-impact relationship
  // if "tr_digit" is confirmed for all cells in "tr_cells", then trigger *this
  bool add_triggers(set<solv_cell*>* tr_thesis_cells, const int tr_thesis_digit, const uint level, const uint level_bits);
 
  // remove a trigger and all its references from the node
	bool remove_trigger(fp_trigger_p _trigger);
	bool remove_all_triggers();
	// add an impact to the impact set
	bool add_impact(fp_trigger_p impact, const uint level);
	// remove an impact from the node
	bool remove_impact(fp_trigger_p _impact);
	
	friend bool fp_gap(const fp_node*, const fp_node*, const uint level_bits, const uint restrict_level, const uint level = 0);
  friend ostream& operator<<(ostream& os, const fp_node& n);
  friend bool operator<(const fp_node& left, const fp_node& right);

  operator int() const;
};




ostream& operator<<(ostream& os, const fp_node_set& s);
ostream& operator<<(ostream& os, const fp_trigger_set& s);
ostream& operator<<(ostream& os, const fp_impact_set& s);



#endif
