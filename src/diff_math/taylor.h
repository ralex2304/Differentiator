#ifndef TAYLOR_H_
#define TAYLOR_H_

#include <assert.h>

#include "../diff_objects.h"
#include "../interface.h"
#include "../utils/statuses.h"
#include "differentiation.h"
#include "../tex/tex.h"
#include "simplification.h"

#include "../config.h"
#include TREE_INCLUDE
#include "../TreeAddon/TreeAddon.h"

Status::Statuses diff_do_taylor(DiffData* diff_data, const size_t degree, const double point);

#endif //< #ifndef TAYLOR_H_
