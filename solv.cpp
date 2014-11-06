#include "solv_rules.h"

int main(int argc, char** argv){
	solv_sudoku* su;

	// read a sudoku grid from a filename provided by the user, or stdin

	const char* filename = (argc>1 ? argv[1] : "stdin");
	printf("reading file %s\n", filename);
	su = new solv_sudoku(filename, 0);
	su->print();

	// create rule-objects to use with the sudoku grid
	solv_rule* floodrule = new solv_rule(flood);
	solv_rule* tcarule = new solv_rule(tca);
//	solv_rule* tcarule = new solv_rule(bigraph);
//	solv_rule* tcarule = new solv_rule(ebigraph);
	solv_rule* eliminaterule = new solv_rule(eliminate);
	solv_rule* locaterule = new solv_rule(locate);
	solv_rule* alignrule = new solv_rule(alignment);
	solv_rule* intersectrule = new solv_rule(group_intersect);

	// apply the rule objcts in the given order
	int count = 0;
	printf("applying flood\n");
	while(su->applyrule(floodrule, LVL_ALL)) count++;
	printf("flood rounds %d\n", count);
	su->print();

	count = 0;
	printf("applying eliminate\n");
	while(su->applyrule(eliminaterule, LVL_ALL)) count++;
	printf("eliminate rounds %d\n", count);
	su->print();
	
	count = 0;
	printf("applying locate\n");
	while(su->applyrule(locaterule, LVL_ALL)) count++;
	printf("locate rounds %d\n", count);
	su->print();

	count = 0;
	printf("applying group_intersect\n");
	while(su->applyrule(intersectrule, LVL_ALL)) count++;
	printf("group_intersect rounds %d\n", count);
	su->print();

	count = 0;
	printf("applying alignment\n");
	while(su->applyrule(alignrule, LVL_ALL)) count++;
	printf("alignment rounds %d\n", count);
	su->print();

/*
	count = 0;
	printf("applying tca\n");
	while(su->applyrule(tcarule, LVL_ALL)) count++;
	printf("tca rounds %d\n", count);
	su->print();
*/

  cout << "cleaning up..." << endl;
	delete tcarule;
	delete floodrule;
	delete su;
  cout << "done. Goodbye." << endl;
}
