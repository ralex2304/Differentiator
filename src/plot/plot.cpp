#include "plot.h"

static Status::Statuses diff_plot_write_header_(DiffData* diff_data, FILE* file);

static Status::Statuses diff_plot_eval_vals_(DiffData* diff_data, FILE* file, Tree** trees,
                                             const size_t trees_num, const PlotRange range);

static Status::Statuses diff_plot_make_plot_line_(DiffData* diff_data, FILE* file, const char** titles,
                                                  const size_t trees_num);

static Status::Statuses diff_plot_call_gnuplot_(DiffData* diff_data);


#define PRINTF_(...)    if (!file_printf(file, __VA_ARGS__))  \
                            return Status::OUTPUT_ERROR

Status::Statuses diff_plot(DiffData* diff_data, Tree** trees, const char** titles, const size_t trees_num) {
    assert(diff_data);
    assert(trees);
    assert(titles);
    assert(trees_num > 0);

    PlotRange range = {};

    STATUS_CHECK(interface_ask_plot_params(&range.min, &range.max));
    range.step = (range.max - range.min) / 1000; //< optimal resolution

    char filename[diff_data->MAX_PATH_LEN * 2] = {};

    snprintf(filename, diff_data->MAX_PATH_LEN * 2, "%s/plot.plt", diff_data->tex_dir);

    FILE* file = nullptr;
    if (!file_open(&file, filename, "wb"))
        return Status::OUTPUT_ERROR;

    STATUS_CHECK(diff_plot_write_header_(diff_data, file), file_close(file));

    STATUS_CHECK(diff_plot_eval_vals_(diff_data, file, trees, trees_num, range), file_close(file));

    STATUS_CHECK(diff_plot_make_plot_line_(diff_data, file, titles, trees_num),  file_close(file));

    if (!file_close(file))
        return Status::OUTPUT_ERROR;

    STATUS_CHECK(diff_plot_call_gnuplot_(diff_data));

    return Status::NORMAL_WORK;
}

static Status::Statuses diff_plot_write_header_(DiffData* diff_data, FILE* file) {
    assert(diff_data);
    assert(file);

    char filename[diff_data->MAX_PATH_LEN * 2] = {};

    snprintf(filename, diff_data->MAX_PATH_LEN * 2, "%s/plot.png", diff_data->tex_dir);

    PRINTF_("set term png\n"
            "set output \"%s\"\n"
            "set xlabel \"X\"\n"
            "set ylabel \"Y\"\n"
            "set grid\n", filename);

    return Status::NORMAL_WORK;
}

static Status::Statuses diff_plot_eval_vals_(DiffData* diff_data, FILE* file, Tree** trees,
                                             const size_t trees_num, const PlotRange range) {
    assert(diff_data);
    assert(file);
    assert(trees);

    double x_tmp_val = diff_data->vars.arr[diff_data->vars.argument].val;

    for (size_t i = 0; i < trees_num; i++) {
        PRINTF_("\n$Data%zu <<EOD\n", i);

        for (double val = range.min; val <= range.max; val += range.step) {
            diff_data->vars.arr[diff_data->vars.argument].val = val;

            double res = NAN;

            STATUS_CHECK(diff_eval(diff_data, &trees[i]->root, &res));
            assert(!isnan(res));

            PRINTF_("%lf %lf\n", val, res);
        }

        PRINTF_("EOD\n\n");
    }

    diff_data->vars.arr[diff_data->vars.argument].val = x_tmp_val;

    return Status::NORMAL_WORK;
}

static Status::Statuses diff_plot_make_plot_line_(DiffData* diff_data, FILE* file, const char** titles,
                                                  const size_t trees_num) {
    assert(diff_data);
    assert(file);
    assert(titles);

    PRINTF_("\nplot ");

    const char* colors[] = {"red",
                            "blue",
                            "green",
                            "purple",
                            "yellow"};
    assert(trees_num < sizeof(colors) / sizeof(*colors));

    for (size_t i = 0; i < trees_num; i++) {
        PRINTF_("$Data%zu smooth csplines title \"%s\" lc rgb \"%s\"", i, titles[i], colors[i]);

        if (i + 1 < trees_num)
            PRINTF_(", ");
    }

    PRINTF_("\n");

    return Status::NORMAL_WORK;
}

static Status::Statuses diff_plot_call_gnuplot_(DiffData* diff_data) {
    assert(diff_data);

    char command[diff_data->MAX_PATH_LEN * 2] = {};

    snprintf(command, diff_data->MAX_PATH_LEN * 2, "gnuplot %s/plot.plt", diff_data->tex_dir);

    if (system(command) != 0)
        return Status::OUTPUT_ERROR;

    return Status::NORMAL_WORK;
}
