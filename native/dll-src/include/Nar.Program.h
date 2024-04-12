#ifndef PROJECT_NAR_PROGRAM_NAR_PROGRAM_H
#define PROJECT_NAR_PROGRAM_NAR_PROGRAM_H

#include <nar.h>

// to_msg_arg can be NAR_INVALID_OBJECT if msg is constant (zero-arity function)
typedef void (*cmd_callback_fn_t)(nar_runtime_t rt, nar_object_t to_msg_arg, void* cmd_state);

// cmd_exec_fn_t() function should call callback() function EXACTLY ONCE
typedef void (*cmd_exec_fn_t)(
        nar_runtime_t rt, nar_object_t data, cmd_callback_fn_t callback, void* cmd_state);

typedef void (*exit_callback_fn_t)(nar_runtime_t rt, int exit_code);

nar_object_t (*cmd_new_fn_t)(
        nar_runtime_t rt, cmd_exec_fn_t exec, nar_object_t to_msg, nar_object_t arg);

#endif //PROJECT_NAR_PROGRAM_NAR_PROGRAM_H
