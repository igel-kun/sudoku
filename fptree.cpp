/***************************************************
 * fptree.cpp
 * lib to manage force-propagation-trees for sudoku solution
 * by M.Weller
 **************************************************/
#include "fptree.h"
#include "sudoku.h"
#include "solv_rules.h"

bool trigger_triggered(const fp_trigger* trigger, const uint level_bits){
	if(level_allowed(trigger->level,level_bits)){
		for(fp_node_set::iterator i = trigger->node_set->begin(); i != trigger->node_set->end(); i++)
			if(!((*i)->is_triggered())) return false;
		return true;
	} else {
		printf("disallowing level %x [levelbits %x]\n", trigger->level, level_bits);
		return false;
	}
}


fp_trigger::operator int() const{
  int result = 0;

  for(fp_node_set::const_iterator i = node_set->begin(); i != node_set->end(); ++i)
    if(*i) result += (int)(**i);

  if(owner) result += (int)(*owner);
  return result + level;
}

/***************** force propagation trees **********************/

// force propagation tree node: a cell and one of its 
// possibilities/non-possibilities in a sudoku grid. 
// basically a hypothesis of placing/not placing a digit there
void fp_node::init(const fp_trigger_set* _triggers, const fp_impact_set* _impacts){
		triggered = false;
		if(_triggers) triggers = new fp_trigger_set(*_triggers);
				else  triggers = new fp_trigger_set();
		if(_impacts) impacts = new fp_impact_set(*_impacts);
				else impacts = new fp_impact_set();
	}

// constructor
fp_node::fp_node(solv_cell* _cell, const int _thesis):cell(_cell),thesis(_thesis){
	init(NULL, NULL);
}

// copy constructor
fp_node::fp_node(const fp_node& fpnode):cell(fpnode.cell),thesis(fpnode.thesis){
	init(fpnode.triggers, fpnode.impacts);
}
	
// destructor
// 1. remove references to this node from all triggers
// 2. free its memories
fp_node::~fp_node(){
//	printf("destroying %p (%d,%d,%d) [%d triggers, %d impacts]:\n", this, cell->get_x(), cell->get_y(), thesis, triggers->size(), impacts->size());


	// erase all nodes' impacts that would trigger any trigger of this thesis 
	for(fp_trigger_set::iterator i = triggers->begin(); i != triggers->end(); i++)
		if(!remove_trigger(*i))
      diewith("something is fishy [triggers]..."<<endl);

	// erase all nodes' triggers that this thesis has an impact on
	// note: this removes whole triggers because if one node of a trigger is
	// 		 impossible, the whole trigger is
	for(fp_impact_set::iterator i = impacts->begin(); i != impacts->end();){
	  fp_impact_p imp = *i;
    ++i;

		if(!imp->owner->remove_trigger(imp))
      diewith("something is fishy [impacts]..."<<endl);

		delete imp->node_set;
		delete imp;
	}

	delete triggers;
	delete impacts;
}

int fp_node::get_thesis() const{
	return thesis;
}
bool fp_node::is_triggered() const{
	return triggered;
}

// add a trigger to the trigger set and return it
// return a pointer to the trigger who contains
// the given fp_node_set
fp_trigger_p fp_node::add_trigger(
                          fp_node_set* nodes,
                          const uint level,
                          const uint level_bits){
	if(!nodes) return false;
	// dont add triggers to a triggered node
	if(triggered) return false;
	// check if the trigger is triggerable [whether it contains NULL]
	if(nodes->find(NULL) != nodes->end()) {
		delete nodes;
    nodes = NULL;
		return NULL;
	}
  dbgout << "nodes are " << *nodes << endl;
  // remove all triggered nodes
	for(fp_node_set::iterator i = nodes->begin(); i != nodes->end();)
		if(*i){
      if((*i)->is_triggered()){
        fp_node_set::iterator j = i++;
        nodes->erase(j);
      } else ++i;
    }

	fp_trigger* tr = new fp_trigger(nodes, this, level);

  // if the node set is empty (the empty set triggers *this), then trigger *this
  if(nodes->empty()) {
    dbgout << "got empty trigger set, triggering " << *this << endl;
    set_trigger(level_bits);
    return tr;
  }

	pair<fp_trigger_set::iterator, bool> result = triggers->insert(tr);
  
	if(!result.second) {
		// the trigger is in the list already
		delete tr;
		dbgout << "trigger exists: " << **(result.first) << endl;

//		return *(result.first);
    return NULL;
	} else {
    dbgout << "adding trigger " << *tr <<  " to thesis " << *this << endl;

		// add symmectric impacts
		for(fp_node_set::iterator i = nodes->begin(); i != nodes->end(); ++i)
			(*i)->add_impact(tr, level);

    dbgout << "done adding trigger" << endl;
		return tr;
	}
}


// add a single fp_node to the trigger set
fp_trigger_p fp_node::add_trigger(fp_node* _tr_node,
                                  const uint level,
                                  const uint level_bits){
	if(!_tr_node) return NULL;

  dbgout << "adding single trigger " << *_tr_node << " to " << *this << endl;

	fp_node_set* node_set = new fp_node_set();
	node_set->insert(_tr_node);
		
	return add_trigger(node_set, level, level_bits);
}


// convinience function for adding a triggers-impact relationship
// if "tr_digit" is confirmed for all cells in "tr_cells", then trigger *this
bool fp_node::add_triggers(
                    solv_set* tr_cells, 
                    const int tr_digit, 
                    const uint level, 
                    const uint level_bits){

	fp_node_set* fp_group = new fp_node_set();
  
	for(solv_set::iterator i = tr_cells->begin(); i != tr_cells->end(); i++){
    const solv_cell& cell = **i;
		fp_group->insert(cell[tr_digit]);
  }
	return add_trigger(fp_group, level, level_bits);
}


// remove a trigger and all its references from the node
bool fp_node::remove_trigger(const fp_trigger* _trigger){
	if(triggers->find(_trigger) == triggers->end())
		diewith("trigger " << *_trigger << " is not in the trigger-list: " << *triggers << endl);

 // remove all impacts pointing to _trigger
  for(fp_node_set::iterator i = _trigger->node_set->begin(); i != _trigger->node_set->end(); ++i)
    if(!(*i)->remove_impact(_trigger))
      diewith("it appears " << **i << " doesn't trigger " << *_trigger << " after all...");

  //dbgout << "removing "<< *_trigger << " from " << *this << " as requested" << endl;
#warning fixme: this is a memory leak since I forget the pointer to the trigger and can no longer free it
  return triggers->erase(_trigger);
} 

// add an impact to the impact set
bool fp_node::add_impact(fp_trigger_p impact, const uint level){
//	pair<fp_trigger_set::iterator, bool> result = impacts->insert(impact);
  return (impacts->insert(impact)).second;
}
// remove an impact and all its references from the node
bool fp_node::remove_impact(fp_trigger_p _impact){
	return (impacts->erase(_impact) > 0);
}

solv_cell* fp_node::get_cell() const{
	return cell;
}


// (
int indent = 0;
void fp_node::set_trigger(const uint level_bits){
	if(triggered) return;
	if(!thesis) diewith("triggering 'invalid_sudoku'"<<endl);

  if(thesis > 0) cout << "found that " << *this << endl;
	dbgout << indent++ << ": triggering " << *this << endl;

	triggered = true;

  // remove the opposite thesis
	cell->remove_thesis(-thesis, level_bits);

	if(thesis > 0) cell->set_content(thesis, level_bits);

  // remove this from the triggers of all its impacts
  while(!impacts->empty()){
    fp_impact_p imp = *(impacts->begin());
    fp_node* node = imp->owner;
 
    // we have to remove and readd the triggers because their hash-value changes
    // when re-adding the triggers, this will be removed automatically since it's triggered
    dbgout << "fixing trigger " << *imp << endl;
    node->remove_trigger(imp);
    node->add_trigger(imp->node_set, imp->level, level_bits);
  }

  // next, remove all its triggers
  while(!triggers->empty()){
    fp_trigger_p tr = *(triggers->begin());
    fp_node* node = tr->owner;
    node->remove_trigger(tr);
  }

	indent--;
	dbgout << indent << ": done triggering" << endl;
}


// returns if from can reach 'to' with a slihtly modified DFS
// restricted means:
// same as fp_gap with the restriction that all triggers
// being followd must have at most one untriggered node 
// (in other words its a cycle in the bigraph [for use with bi_graph]
// to trace non-extended rules [without Gx], forbit to follow LVL_FLOOD rules,
// except if both end-nodes have less then 3 possible numbers
set<const fp_node*> visited;
bool fp_gap(const fp_node* from, const fp_node* to, const uint level_bits, const uint restrict_level, const uint level){
	if(!from || !to) return false;
	dbgout << level << ":\tfinding path from" << *from << " to " << *to << endl;
	if((from == to) || (to->is_triggered())) {
		visited.clear();
		return true;
	} else {
		bool is_marked;
		fp_node* opp = (*from->get_cell())[-from->get_thesis()];
		if(opp)	if(visited.find(opp) != visited.end()) {
				visited.clear();
				return true;
			}
		visited.insert(from);
		for(fp_impact_set::iterator i = from->impacts->begin(); i != from->impacts->end(); i++)
			// only consider triggers of appropriate level
			if(level_allowed((*i)->level, level_bits)){
				// if the node to be triggered is already visited, just continue the loop
				if(visited.find((*i)->owner) == visited.end()) {
					// if any of the nodes of i was not visited or triggered, continue loop
					// this makes sure each trigger is only taken into account if all its nodes 
					// are found to be impacts of the node we started the search from
					is_marked = true;
					if(!(*i)->owner->is_triggered()){
						uint count_untrigg = 0;
						fp_node* the_untrig = NULL;
						for(fp_node_set::iterator j = (*i)->node_set->begin(); j != (*i)->node_set->end(); j++)
							if(!((*j)->is_triggered())) {
								count_untrigg++;
								the_untrig = *j;
								is_marked &= ((visited.find(*j)) != visited.end());
							}

						// restriction implementation [for use with bi_graphs]
						if(restrict_level && is_marked){
							printf("restricted gap: branching okay: %s\n", is_marked?"yes":"no");
							if((count_untrigg > 1)) is_marked = false;
							else{ // if the restriction level is < 2, forbid group_flood rules
								if((restrict_level < 2) && ((*i)->level & LVL_FLOOD) > 0) {
									if(the_untrig) {
										if((the_untrig->get_cell()->count_poss() > 2) || 
											((*i)->owner->get_cell()->count_poss() > 2)) // unless it is bivalued
											is_marked = false;
//										printf("okay to branch: %s, posses: %i, %i\n", is_marked?"yes":"no",the_untrig->get_cell()->count_poss(), (*i)->owner->get_cell()->count_poss());
									} else {printf("uh oh, panic!\n");exit(1);}
                }
              }
						}			
					}
					// if all nodes of the trigger are visited or triggered, it is an impact of the node
					// we started searching from, so continue search from there
					if(is_marked){
						dbgout << "branching: " << **i << endl;
						if(fp_gap((*i)->owner, to, level_bits, restrict_level, level + 1)) return true;
					}
				}
			}
		if(!level) visited.clear();
		return false;
	}
}

ostream& operator<<(ostream& os, const fp_node& n){
  return os << (n.triggered?"*":"") << *(n.cell) << "=" << n.thesis;
}

ostream& operator<<(ostream& os, const fp_node_set& s){
  os << "{";
  for(fp_node_set::const_iterator i = s.begin(); i != s.end(); ++i){
    if(*i) os << **i << " "; else os << "NULL ";
  }
  return os << "}";
}

ostream& operator<<(ostream& os, const solv_set& s){
  os << "{";
  for(solv_set::const_iterator i = s.begin(); i != s.end(); ++i){
    if(*i) os << **i << " "; else os << "NULL ";
  }
  return os << "}";
}

ostream& operator<<(ostream& os, const fp_trigger& t){
	if(level_allowed(t.level, LVL_FLOOD))     os << "F ";
	if(level_allowed(t.level, LVL_ELIMINATE)) os << "E ";
	if(level_allowed(t.level, LVL_LOCATE))    os << "L ";
	if(level_allowed(t.level, LVL_GROUP))     os << "G ";

  trigger_hash ha;
  os << "(" << ha(&t) << ")";

  return os << *t.node_set <<" --> " << *t.owner;
}

ostream& operator<<(ostream& os, const fp_trigger_set& s){
  fp_trigger_set::hasher ha = s.hash_function();

  os << "{";
  for(fp_trigger_set::const_iterator i = s.begin(); i != s.end(); ++i)
    os << "[" << ha(*i) << "]" << **i << " ";
  return os << "}";
}

ostream& operator<<(ostream& os, const fp_impact_set& s){
  os << "{";
  for(fp_impact_set::const_iterator i = s.begin(); i != s.end(); ++i){
    cout << "owner @" << (*i)->owner << endl;
    if( (*i)->owner) os << *((*i)->owner) << " "; else os << "NULL ";
  }
  return os << "}";
}

bool operator<(const fp_node& left, const fp_node& right){
  if(*(left.cell) < *(right.cell)) return true;
  if(*(right.cell) < *(left.cell)) return false;

  return left.thesis < right.thesis;
}

fp_node::operator int() const{
  if(cell){
    return abs((int)(*cell) + thesis);
  } else {
    return abs(thesis);
  }
}


