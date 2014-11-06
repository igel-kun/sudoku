#ifndef ALIGN_H
#define ALIGN_H

#include "solv_rules.h"

// Apply align to a bipartite graph starting at vertex "node".
// This is equal to finding a perfect matching in the dist-2 NH of "node".
bool cell_align(const solv_set* group, const solv_cell* node, const uint level_bits);
bool digit_align(const solv_set* group, const uint digit, const uint level_bits);

#endif
