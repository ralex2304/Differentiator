#ifndef RECURSIVE_FALL_H_
#define RECURSIVE_FALL_H_

#include <assert.h>
#include <stdlib.h>

#include "../utils/statuses.h"
#include "../diff_objects.h"

#include "../config.h"
#include TREE_INCLUDE
#include "../TreeAddon/TreeAddon.h"

Status::Statuses recursive_fall(DiffData* diff_data, char* text);


#endif //< #ifndef RECURSIVE_FALL_H_
