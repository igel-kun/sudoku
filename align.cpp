/*********************************************
 * Code by M.Weller
 *********************************************
*/

#include "align.h"

// Apply align to a bipartite graph starting at vertex "node".
// This is equal to finding a perfect matching in the dist-2 NH of "node"
// or "digit".



//
// For example, we start at "digit 1", its neighbors are all "cells with 1".
// Their neighbors are all digits that occur only in cells containing 1.
// 
// Assume there are as many digits as there are cells in this dist-2 NH around
// "digit 1". If any digit other than "digits" is put into any of the "cells",
// then we cannot fit "digits" into the cells anymore.

bool digit_align(const solv_set* group, const uint digit, const uint level_bits){
  if(!digit || !group) return false;
  if(group->empty()) return false;

  const uint num_digits = (*(group->begin()))->getnum_digits();
  if(!num_digits) return false;

  unordered_set<uint> digits;
  unordered_set<const solv_cell*> cells;

  // compute the set of cells containing "digit"
  dbgout << "cells with " << digit << ": ";
  for(solv_set::const_iterator cell = group->begin(); cell != group->end(); ++cell){
    if((**cell)[digit]) {
      cells.insert(*cell);
      dbgout << **cell << " ";
    }
  }
  dbgout << endl;

  // compile a list of digits that do not occur outside of "cells"
  for(uint i = num_digits; i != 0; --i) digits.insert(i);
  for(solv_set::const_iterator cell = group->begin(); cell != group->end(); ++cell)
    if(cells.find(*cell) == cells.end())
      for(unordered_set<uint>::const_iterator i = digits.begin(); i != digits.end();)
        if(! (**cell)[*i]){
          unordered_set<uint>::const_iterator j = i++;
          digits.erase(j);
        } else ++i;

  dbgout << "found " << digits.size() << " digits and " << cells.size() << " cells" << endl;

  // see whether the sizes match up
  if((digits.size() > 1) && (digits.size() == cells.size())){
    bool result = false;
    dbgout << "found match" << endl;

    // we want to trigger -i for all i that are _not_ in digits, so collect this set first
    list<uint> non_digits;
    for(uint i = num_digits; i != 0; --i)
      if(digits.find(i) == digits.end())
        non_digits.push_back(i);

    // trigger -i for all i that are not in "digits"
    for(unordered_set<const solv_cell*>::const_iterator cell = cells.begin(); cell != cells.end(); ++cell)
      for(list<uint>::const_iterator i = non_digits.begin(); i != non_digits.end(); ++i)
        if((**cell)[*i]){ // cell contains i which is not in digits, so trigger -i if possible
          fp_node* fpnode = (**cell)[-(*i)];
          if(fpnode){
            fpnode->set_trigger(level_bits);
            result = true;
          } else diewith("invalid sudoku found via cell_align at "<<**cell<<endl);
        }

    return result;
  } else return false;
}


//
// Likewise, starting at a cell C, its neighbors are all possible digits for C.
// Their neighbors X are all cells whose digits are also in C. Assuming there
// are as many cells as digits, none of the digits can occur in a cell outside X
// since otherwise there are not enough cells to fit all digits.

bool cell_align(const solv_set* group, const solv_cell* node, const uint level_bits){

  if(!node || !group) return false;

  // if the cell has just one number, the whole thing doesn't make sense
  if(node->get_content()) return false;
  
  const uint num_digits = node->getnum_digits();
  if(!num_digits) return false;

  unordered_set<uint> digits;
  unordered_set<const solv_cell*> cells;

  for(uint digit = num_digits; digit != 0; --digit)
    if((*node)[digit]) digits.insert(digit);


  if(DEBUG){
    dbgout << "cells with subset of (";
    for(unordered_set<uint>::const_iterator i = digits.begin(); i != digits.end(); ++i)
      dbgout << *i << " ";
    dbgout << "):" << endl;
  }

  for(solv_set::const_iterator cell = group->begin(); cell != group->end(); ++cell){
    bool candidate = true;

    // if the cell contains a digit that is not in digits, then it's not a candidate
    for(uint digit = num_digits; digit != 0; --digit)
      if(((**cell)[digit]) && (digits.find(digit) == digits.end())) candidate = false;

    if(candidate){
      dbgout << **cell << " ";
      cells.insert(*cell);
    }
  }
  dbgout << endl;
    
  dbgout << "found " << digits.size() << " digits and " << cells.size() << " cells" << endl;

  // see whether the sizes match up
  if(digits.size() == cells.size()){
    bool result = false;
    dbgout << "found match" << endl;
 
    // for all cells in "group" that are not in "cells", remove all "digits"
    // from them (aka trigger -i for all i in "digits")
    for(solv_set::const_iterator cell = group->begin(); cell != group->end(); ++cell)
      if(cells.find(*cell) == cells.end()){ // got a cell that's not in "cells"
        for(unordered_set<uint>::const_iterator i = digits.begin(); i != digits.end(); ++i)
          if((**cell)[*i]){ // found an i from "digits" in this cell, so trigger -i
            fp_node* fpnode = (**cell)[-(*i)];
            if(fpnode){
              fpnode->set_trigger(level_bits);
              result = true;
            } else diewith("invalid sudoku found via cell_align at " << **cell << endl);
          }
      }


    return result;
  } else return false;
}





