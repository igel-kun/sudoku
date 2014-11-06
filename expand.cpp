/********************************************
 * expand the eppstein-ruleset
 * by M.Weller
 ********************************************/



#include "sudoku.h"
#include "gen_rules.h"
#include <iostream>
#include <algorithm>

bool extend(const uint x, const uint y, gen_sudoku* s){
	bool applicable = false;
	uint num_digits = s->getnum_digits();
	
	vtree_list* group;
	vtree_list* group2;
	vtree_list* isect;
	
	insert_iterator<vtree_list>* iter;
					
	// cannot apply to empty cell
	if(!s->get_node(x,y)->get_content()) return false; 

	for(uint digit = 1; digit <= num_digits; digit++){
		applicable = false;
		// for each group, count all possibilities of the digit in 
		// intersecting groups which are not in the current group. 
		// if this number is one, we got a link

		for(uint j = 0; j < 3; j++){
			group = s->getvgroup(x,y,j);
			for(uint i = 0; i < 3; i++) if(i != j)
				for(uint n = 0; n < num_digits; n++){
					isect = new vtree_list();
					iter = new insert_iterator<vtree_list>(*isect, isect->begin());
						
					group2 = s->getvgroup(x,y,i);
					set_intersection(group->begin(), group->end(), 
									 group2->begin(), group2->end(), *iter);
					if(!isect->empty()){
						delete iter;
						iter = new insert_iterator<vtree_list>(*isect, isect->begin());
						set_difference(group2->begin(), group2->end(),
									   group->begin(), group->end(), *iter);
					}
					delete iter;
					delete isect;
				}
			
			delete group;
		}
		if(applicable) break;
	}
	return applicable;
}

int main(int argc, char** argv){
	gen_rule extend_rule(extend);	
	bool applicable;

	gen_sudoku s(9);
	cin >> s;
	applicable = s.applyrule(&extend_rule);
	if(applicable) {
		cout << s;
		return 1;
	} else return 0;
}
