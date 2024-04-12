#ifndef PACKAGE_H
#define PACKAGE_H

#include <nar.h>
#include <nar-package.h>
#include <stdbool.h>
#include <vector.h>

extern nar_t *nar;

// to_msg_arg can be NAR_INVALID_OBJECT if msg is constant (zero-arity function)
typedef void (*cmd_callback_fn_t)(nar_runtime_t rt, nar_object_t to_msg_arg, void* cmd_state);

// cmd_exec_fn_t() function should call callback() function EXACTLY ONCE
typedef void (*cmd_exec_fn_t)(
        nar_runtime_t rt, nar_object_t data, cmd_callback_fn_t callback, void* cmd_state);

typedef void (*exit_callback_fn_t)(nar_runtime_t rt, int exit_code);

typedef struct {
    bool alive;
    int exit_code;
    vector_t *cmds;
    vector_t *msgs;
    void* model;
    void *update;
    void *subscribe;
    void *view;
    void *present;
    exit_callback_fn_t exit_callback;
} state_t;

nar_object_t cmd_new(nar_runtime_t rt, cmd_exec_fn_t exec, nar_object_t to_msg, nar_object_t arg);
void program_execute(nar_runtime_t rt, nar_object_t program, exit_callback_fn_t exit);

#endif //PACKAGE_H
