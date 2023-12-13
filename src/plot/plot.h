#ifndef PLOT_H_
#define PLOT_H_

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>

#include "../utils/statuses.h"
#include "../diff_objects.h"
#include "../interface.h"
#include "../diff_math/eval.h"
#include "../config.h"
#include TREE_INCLUDE
#include "../TreeAddon/TreeAddon.h"

#include "../file/file.h"

struct PlotRange {
    double min = NAN;
    double max = NAN;
    double step = NAN;
};

Status::Statuses diff_plot(DiffData* diff_data, Tree** trees, const char** titles, const size_t trees_num);

#endif //< #ifndef PLOT_H_
