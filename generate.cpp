#include "gen_rules.h"

int main(int argc, char** argv){
	char* filename = (char*)calloc(256,1);

	if(!(argc > 1)) filename="test_gen.sud"; else filename=argv[1];
	printf("reading file %s\n", filename);
	gen_sudoku* su = new gen_sudoku(filename);
	su->print();

	gen_rule* floodrule = new gen_rule(reverse_flood);
	gen_rule* eliminaterule = new gen_rule(reverse_eliminate);
	gen_rule* locaterule = new gen_rule(reverse_locate);
	
	uint digits = su->getnum_digits();
	for(uint i = 0; i < digits; i++)
		for(uint j = 0; j < digits; j++)
			su->applyrule(i,j,floodrule);
	printf("applied reverse-flood\n");
	su->print();
	
	while(su->applyrule(eliminaterule));
	printf("applied reverse-eliminate\n");
	su->print();
	
	while(su->applyrule(locaterule));
	printf("applied reverse-locate\n");
	su->print();
	
	delete locaterule;
	delete eliminaterule;
	delete floodrule;
	delete su;
}
