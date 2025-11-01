#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define NOB_IMPLEMENTATION
#include "vendor/nob/nob.h"

#define CXX           "gcc"
#define MAIN_EXE_NAME "exe"
#define BUILD_DIR     "build"
#define SRC_DIR       "src"

#define FLAGS "-Wall", "-Wextra", "-Werror", "-pedantic"

Nob_File_Paths src_paths = {0};
Nob_Cmd build_cmd        = {0};
Nob_Cmd run_cmd          = {0};

#ifdef TEST_CMD
#error "test_cmd not implemented yet"
#endif

void usage(char *);

int main(int argc, char **argv) {
    bool err           = false;
    bool push_run_args = false;
    bool should_build  = false;
    bool should_run    = false;

    NOB_GO_REBUILD_URSELF(argc, argv);

    if (!nob_mkdir_if_not_exists(BUILD_DIR))
        return EXIT_FAILURE;

    if (argc < 2) {
        usage(argv[0]);
        nob_log(NOB_ERROR, "Not Enough args: Expected 2, found %d", argc);
        return EXIT_FAILURE;
    }

    if ((err = nob_read_entire_dir(SRC_DIR, &src_paths)) != true)
        return EXIT_FAILURE;

    // Make build command
    nob_cmd_append(&build_cmd, CXX, FLAGS);
    nob_cmd_append(&build_cmd, "-o", "./" BUILD_DIR "/" MAIN_EXE_NAME);

    for (size_t i = 0; i < src_paths.count; ++i) {
        const char *path = src_paths.items[i];
        if (strcmp(path, ".") == 0 || strcmp(path, "..") == 0)
            continue;
        char *path_real = nob_temp_sprintf(SRC_DIR "/%s", path);
        nob_cmd_append(&build_cmd, path_real);
    }

    // Make run command
    nob_cmd_append(&run_cmd, "./" BUILD_DIR "/" MAIN_EXE_NAME);

    // Parse arguments
    for (size_t i = 0; i < (size_t)argc; ++i) {
        char *arg = argv[i];

        // Run arguments
        if (strcmp(arg, "--") == 0) {
            push_run_args = true;
            continue;
        }

        if (push_run_args) {
            nob_cmd_append(&run_cmd, arg);
            continue;
        }

        //
        if (strcmp(arg, "build") == 0)
            should_build = true;

        if (strcmp(arg, "run") == 0)
            should_run = true;
    }

    if (should_build) {
        if (!nob_cmd_run(&build_cmd))
            return EXIT_FAILURE;
    }

    if (should_run) {
        if (!nob_cmd_run(&run_cmd))
            return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

void usage(char *prog) {
    nob_log(NOB_INFO, "USAGE   : %s [COMMAND(S)] -- [RUN ARGS]", prog);
    nob_log(NOB_INFO, "COMMANDS:");
    nob_log(NOB_INFO, "    build: build the project defined in nob.c");
    nob_log(NOB_INFO, "    run  : run the project defined in nob.c");
    nob_log(NOB_INFO, "RUN ARGS:");
    nob_log(NOB_INFO, "    These args");
}
