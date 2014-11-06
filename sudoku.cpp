/*****************************************
 * general header file for sudoku puzzles
 * and rules wrapper
 * by M.Weller
 ****************************************/
#ifndef sudoku_cpp
#define sudoku_cpp

#include "sudoku.h"

void sudoku_cell::init(const uint _num_digits, const uint _x, const uint _y, const uint _content){
	x = _x; 
	y = _y;
	num_digits = _num_digits;
	content = _content;
}
	
// constructor
sudoku_cell::sudoku_cell(const uint _num_digits, const uint _x, const uint _y, const uint _content){
	init(_num_digits, _x, _y, _content);
}
// copy constructor
sudoku_cell::sudoku_cell(const sudoku_cell& cell){
	init(cell.num_digits, cell.x, cell.y, cell.content);
}
// destructor
sudoku_cell::~sudoku_cell(){
}	
// user interface
uint sudoku_cell::get_content() const{
	return content;
}

void sudoku_cell::set_content(const uint _content){
	content = _content;
}
void sudoku_cell::remove_content(){
	set_content(0);
}
uint sudoku_cell::get_num_digits()const{
	return num_digits;
}
uint sudoku_cell::get_x() const{
	return x;
}
uint sudoku_cell::get_y() const{
	return y;
}

uint sudoku::get_index(const uint x, const uint y) const{
	return y * num_digits + x;
}
void sudoku::init(const uint digits, const vector<sudoku_cell*>* copy_grid){
	num_digits = digits;
	grid = new vector<sudoku_cell*>(num_digits * num_digits);
	for(uint i = 0; i < digits; i++)
		for(uint j = 0; j < digits; j++)
			if(copy_grid)
				(*grid)[get_index(i,j)] = new sudoku_cell(*((*copy_grid)[get_index(i,j)]));
			else 
				(*grid)[get_index(i,j)] = new sudoku_cell(digits, i, j, 0);
}

char* sudoku::read_first_line(const char* filename){
	FILE* f;
	bool is_file = false;
	
  dbgout << "trying to open \"" << filename << "\"" << endl;
	if(!strcmp(filename,"stdin")) f = stdin; else {
		f = fopen(filename, "ro");
		is_file = true;
		if(!f) diewith("error opening \"" << filename << "\"" << endl);
	}
	char* s = (char*)malloc(MAX_DIGITS + 1);
	if(!fscanf(f, "%s", s)) diewith("error reading \"" << filename << "\": bad format" << endl);
	if(is_file) fclose(f);
	return s;
}

void sudoku::mem_free(){
	for(uint i = 0; i < num_digits * num_digits; i++)
		delete (*grid)[i];
	delete grid;
}


// constructor
sudoku::sudoku(const uint digits){
	init(digits);
}
// construct from a file
sudoku::sudoku(const char* filename, const bool read_field){
	char* s = read_first_line(filename);

	init(strlen(s));
	dbgout << "number of digits: " << num_digits << endl;
	
	if(read_field) read_from_file(filename, strcmp(filename,"stdin")?NULL:s);
	free(s);
}

// copy constructor
sudoku::sudoku(const sudoku& s){
	init(s.num_digits, s.grid);
}
// destructor
sudoku::~sudoku(){
	mem_free();
}
// write a cell to a position in the grid
bool sudoku::put_cell(const uint x, const uint y, sudoku_cell* cell){
	if((x < num_digits) && (y < num_digits)){
		delete (*grid)[y * num_digits + x];
		(*grid)[y * num_digits + x] = cell;
	} else return false;
	return true;
}

// read a cell from a position in the grid
sudoku_cell* sudoku::get_cell(const uint x, const uint y) const{
	if((x < num_digits) && (y < num_digits))
		return (*grid)[y * num_digits +x];
	else return NULL;
}

// return the squared order [ = how many different digits there are]
uint sudoku::getnum_digits() const{
	return num_digits;
}

// return a set of cells in row y
void sudoku::getrow(const uint y, set<sudoku_cell*>* group) const{
	for(uint i = 0; i < num_digits; i++)
		group->insert((*grid)[get_index(i,y)]);
}
// return a set of cells in the column x
void sudoku::getcolumn(const uint x, set<sudoku_cell*>* group) const{
	for(uint i = 0; i < num_digits; i++)
		group->insert((*grid)[get_index(x,i)]);
}
// return a set of cells in the square n
void sudoku::getsquare(const uint n, set<sudoku_cell*>* group) const{
	uint order = (uint)sqrt((double)num_digits);
	uint sq_x = n % order;
	uint sq_y = n / order;
	for(uint i = 0; i < order; i++)
		for(uint j = 0; j < order; j++)
			group->insert((*grid)[get_index(order * sq_x + i, order * sq_y + j)]);
}

// get the n'th [row, col, square] (dependent on group_nr) 
set<sudoku_cell*>* sudoku::getgroup(const uint n, const uint group_nr) const {
	set<sudoku_cell*>* group = new set<sudoku_cell*>();
	switch(group_nr){
		case 0: getrow(n, group); break;
		case 1: getcolumn(n, group); break;
		case 2: getsquare(n, group); break;
	}
	return group;
}


// get the [row, col, square] (dependent on group_nr) of the cell at (x,y)
set<sudoku_cell*>* sudoku::getgroup(const uint x, const uint y, const uint group_nr) const {
	set<sudoku_cell*>* group = new set<sudoku_cell*>();
	switch(group_nr){
		case 0: getrow(x,y, group); break;
		case 1: getcolumn(x,y, group); break;
		case 2: getsquare(x,y, group); break;
	}
	return group;
}
// return a set of cells in the same row as cell (x,y)
void sudoku::getrow(const uint x, const uint y, set<sudoku_cell*>* group) const{
	getrow(y,group);
}
// return a set of cells in the same column as cell (x,y)
void sudoku::getcolumn(const uint x, const uint y, set<sudoku_cell*>* group) const{
	getcolumn(x, group);
}
// return a set of cells in the same square as cell (x,y)
void sudoku::getsquare(const uint x, const uint y, set<sudoku_cell*>* group) const{
	uint order = (uint)sqrt((double)num_digits);
	uint sq_x = x / order;
	uint sq_y = y / order;
	getsquare(sq_y * order + sq_x, group);
}
// read a sudoku field from a given file with the first line provided in first
void sudoku::read_from_file(const char* filename, const char* first){
	FILE* f;
	bool is_file = false;
	if(!strcmp(filename,"stdin")) f = stdin; else {
		f = fopen(filename, "ro");
		is_file = true;
	}
	dbgout << "reading sudoku data from \"" << filename << "\"" << endl;
	if(!f) diewith("error opening \""<< filename <<"\"" << endl);
	
	uint digits[num_digits][num_digits];
	unsigned char* buffer = (unsigned char*)malloc(num_digits + 1); // insecure, may cause segfault
	for(uint row = 0; row < num_digits; ++row){
		if(first && !row){
      strcpy((char*)buffer, first);
    }else{
      if(!fscanf(f, "%s", (char*)buffer))
        diewith("error reading from \"" << filename << "\"" << endl);
    }
		dbgout << "read \"" << buffer << "\" from \"" << filename << "\"" << endl;
		if(strlen((char*)buffer) < num_digits){
      diewith("error reading \"" << filename << "\": not enough digits in row" << row << endl);
    } else {
				for(uint col = 0; col < num_digits; ++col){
          if((buffer[col] < '0') || (buffer[col] > '0'+num_digits))
            digits[col][row] = 0;
          else digits[col][row] = buffer[col] - '0';
        }
		}
	}
	if(is_file) fclose(f);
	free(buffer);

	for(uint col = 0; col < num_digits; ++col)
		for(uint row = 0; row < num_digits; ++row){
			(*grid)[get_index(col, row)]->set_content(digits[col][row]);
		}
}

// output the grid to the stardard output stream
void sudoku::print() const{
  uint zeroes = 0;
	for(uint j = 0; j < num_digits; j++){
		for(uint i = 0; i < num_digits; i++){
			cout << (char)('0' + get_cell(i,j)->get_content());
      if(!get_cell(i,j)->get_content()) ++zeroes;
    }
    cout << endl;
	}
  cout << zeroes << " empty cells" << endl;
}

bool sudoku::is_valid() const{
	set<sudoku_cell*>* group;
	bool bitfield[num_digits];
	bool result = true;
	for(uint group_nr = 0; group_nr < 3; group_nr++){
		for(uint i = 0; i < num_digits; i++){
			group = getgroup(i, group_nr);
			for(uint i = 0; i < num_digits; i++)
				bitfield[i] = false;
			for(set<sudoku_cell*>::iterator j = group->begin(); j != group->end(); j++)
				bitfield[(*j)->get_content() - 1] = true;
			delete group;
			for(uint i = 0; i < num_digits; i++)
				if(!bitfield[i]) return false;
		}
	}
	return result;
}

// equality on several levels [see above]
bool sudoku::is_equal(const sudoku& s, const uint levels) const{
	// check for permuted digits
	//EQ_PERMUTE_DIGITS
	uint x,y;
	bool result = false;
	if(!result && (levels & EQ_PERMUTE_DIGITS)){
		uint *perm = new uint[num_digits];
		uint content;
		for(uint i = 0; i < num_digits; i++) perm[i] = 0;
		x=y=0;	result = true;
		while((y < num_digits) && result){
			content = s.get_cell(x,y)->get_content();
			if(content){
				content--;
				if(perm[content] == 0)
					perm[content] = get_cell(x,y)->get_content();
				else 
					if(perm[content] != get_cell(x,y)->get_content()) result = false;
			} else
				if(get_cell(x,y)->get_content()) result = false;
			x++;
			if(x == num_digits) { x=0; y++;}
		}
		delete perm;
	}
	//EQ_ROTATE
	if(!result && (levels & EQ_ROTATE)){
		for(uint i = 1; i < 4; i++){
		}
	}
	//EQ_FLIP
	if(!result && (levels & EQ_FLIP)){
	}
	//EQ_TRANSPOSE
	if(!result && (levels & EQ_TRANSPOSE)){
		x=y=0; result = true;
		while((y < num_digits) && result){
			if(s.get_cell(x,y)->get_content() != get_cell(y,x)->get_content()) result = false;
			x++;
			if(x == num_digits) { x=0; y++;}
		}
	}

#warning continue here!
	return result;
}

bool sudoku::operator==(const sudoku& s) const{
	return is_equal(s, 0);
}

ostream& operator<<(ostream& os, const sudoku& s){
	for(uint i = 0; i < s.num_digits; i++){
		for(uint j = 0; j < s.num_digits; j++)
			os << (*s.grid)[s.get_index(i, j)]->get_content();
		os << "\n";
	}
	return os;
}

istream& operator>>(istream& is, sudoku& s){
		uint* digits = NULL;
		char* str = (char*)malloc(MAX_DIGITS + 1); // insecure, may cause segfault
		uint num_digits = 1;

		for(uint y = 0; y < num_digits; y++){
			is >> str;
			dbgout << "read " << str << " from the stream" << endl;
			if(!y) {
				num_digits = strlen(str);
				dbgout << "getting " << sizeof(uint) * num_digits * num_digits << " bytes for digits"<<endl;
				digits = (uint*)malloc(sizeof(uint) * num_digits * num_digits);
			}
			if(strlen(str) < num_digits){
        diewith("error reading stream: not enough digits in a row" << endl);
			} else {
				for(int x = 0; x < (int)num_digits; x++)
					digits[s.get_index(x,y)] = str[x] - '0';
			}
		}
		free(str);

		s.mem_free();
		s.init(num_digits);
		
		for(uint x = 0; x < num_digits * num_digits; x++)
			((*s.grid)[x])->set_content(digits[x]);
		free(digits);
	return is;
}


#endif
