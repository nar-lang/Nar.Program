#ifndef PACKAGE_H
#define PACKAGE_H

#include <nar.h>
#include <nar-package.h>
#include <stdbool.h>
#include <vector.h>
#include <hashmap/hashmap.h>
#include "include/Nar.Program.h"

#define NAR_METADATA_NAR_PROGRAM_STATE "Nar.Program:state"

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
} state_t;

typedef struct {
    nar_object_t to_msg_list;
    nar_program_cmd_exec_fn_t exec;
    void *user_data;
} cmd_t;

typedef struct {
    nar_object_t to_msg_list;
    nar_program_sub_on_fn_t on;
    nar_program_sub_off_fn_t off;
    void *user_data;
} sub_t;

void execute(
        nar_runtime_t rt,
        nar_object_t program,
        nar_program_exit_cb_t exit);

nar_object_t cmd_new(
        nar_runtime_t rt,
        nar_object_t to_msg,
        nar_program_cmd_exec_fn_t cmd_exec,
        void *user_data);

nar_object_t sub_new(
        nar_runtime_t rt,
        nar_object_t to_msg,
        nar_program_sub_on_fn_t on,
        nar_program_sub_off_fn_t off,
        void *user_data);

void flush(nar_runtime_t rt);

#endif //PACKAGE_H
