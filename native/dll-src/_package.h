#ifndef PACKAGE_H
#define PACKAGE_H

#include <nar.h>
#include <nar-package.h>
#include <stdbool.h>
#include <vector.h>
#include "include/Nar.Program.h"

#define NAR_META__Nar_Program__state "Nar.Program:state"
#define NAR_META__Nar_Program__args "Nar.Program:args"
#define NAR_META__Nar_Program__updaters "Nar.Program:updaters"

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
    nar_program_update_cb_fn_t update;
    nar_int_t priority;
} updater_t;

void meta__set_args(nar_runtime_t rt, int argc, char **argv);

int meta__execute(nar_runtime_t rt, nar_object_t program, nar_program_exit_cb_t exit_cb);

nar_bool_t meta__update(nar_runtime_t rt);

void meta__updater_add(nar_runtime_t rt, nar_program_update_cb_fn_t update, nar_int_t priority);

void meta__updater_remove(nar_runtime_t rt, nar_program_update_cb_fn_t update);

nar_object_t meta__cmd_new(nar_runtime_t rt, nar_object_t to_msg, nar_object_t payload,
        nar_program_cmd_exec_fn_t cmd_exec);

nar_object_t meta__sub_new(nar_runtime_t rt, nar_object_t to_msg, nar_object_t payload,
        nar_program_sub_on_fn_t on, nar_program_sub_off_fn_t off);

nar_object_t meta__task_new(nar_runtime_t rt, nar_object_t payload,
        nar_program_task_exec_fn_t task_exec);

nar_object_t native__application(nar_runtime_t rt, nar_object_t app);

nar_object_t native__exit(nar_runtime_t rt, nar_object_t exit_code);

nar_object_t native__args(nar_runtime_t rt);

nar_object_t native__Cmd_map(nar_runtime_t rt, nar_object_t f, nar_object_t cmds);

nar_object_t native__Sub_map(nar_runtime_t rt, nar_object_t f, nar_object_t subs);

nar_object_t native__Task_succeed(nar_runtime_t rt, nar_object_t result);

nar_object_t native__Task_fail(nar_runtime_t rt, nar_object_t error);

nar_object_t native__Task_andThen(nar_runtime_t rt, nar_object_t nextTask, nar_object_t task);

nar_object_t native__Task_onError(nar_runtime_t rt, nar_object_t nextTask, nar_object_t task);

nar_object_t native__Task_attempt(nar_runtime_t rt, nar_object_t toMessage, nar_object_t task);

#endif //PACKAGE_H
