#ifndef PACKAGE_H
#define PACKAGE_H

#include <nar.h>
#include <nar-package.h>
#include <stdbool.h>
#include <vector.h>
#include "include/Nar.Program.h"

#define NAR_META__Nar_Program__state "Nar.Program:state"
#define NAR_META__Nar_Program__args "Nar.Program:args"

extern nar_t *nar;

typedef struct {
    bool alive;
    int exit_code;
    vector_t *cmds;
    vector_t *msgs;
    vector_t *subs;
    nar_serialized_object_t model;
    nar_serialized_object_t update;
    nar_serialized_object_t subscribe;
    nar_serialized_object_t view;
    nar_serialized_object_t present;
    nar_program_exit_cb_t exit_callback;
    int argc;
    char **argv;
} state_t;

typedef struct {
    int argc;
    char **argv;
} args_t;

typedef struct {
    nar_object_t to_msg_list;
    nar_program_sub_on_fn_t on;
    nar_program_sub_off_fn_t off;
    nar_object_t payload;
} sub_t;

void set_args(nar_runtime_t rt, int argc, char **argv);

int execute(
        nar_runtime_t rt,
        nar_object_t program,
        nar_program_main_update_cb_t update_cb,
        nar_program_exit_cb_t exit_cb);

nar_object_t cmd_new(
        nar_runtime_t rt,
        nar_object_t to_msg,
        nar_object_t payload,
        nar_program_cmd_exec_fn_t cmd_exec);

nar_object_t sub_new(
        nar_runtime_t rt,
        nar_object_t to_msg,
        nar_object_t payload,
        nar_program_sub_on_fn_t on,
        nar_program_sub_off_fn_t off);

nar_object_t task_new(
        nar_runtime_t rt,
        nar_object_t payload,
        nar_program_task_exec_fn_t task_exec);

nar_bool_t flush(nar_runtime_t rt);

#endif //PACKAGE_H
