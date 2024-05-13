#ifndef NAR_PROGRAM_H
#define NAR_PROGRAM_H

#include <nar.h>

#define NAR_META_NAR_PROGRAM_EXECUTE "Nar.Program:execute"
#define NAR_META_NAR_PROGRAM_CMD_NEW "Nar.Program:cmd_new"
#define NAR_META_NAR_PROGRAM_SUB_NEW "Nar.Program:sub_new"
#define NAR_META_NAR_PROGRAM_FLUSH "Nar.Program:flush"

typedef void (*nar_program_exit_cb_t)(
        nar_runtime_t rt,
        nar_int_t exit_code);

typedef void (*nar_program_execute_fn_t)(
        nar_runtime_t rt,
        nar_object_t program,
        nar_program_exit_cb_t exit);

typedef struct {
    nar_serialized_object_t to_msg_list;
} nar_cmd_state_t;

typedef void (*nar_program_cmd_resolve_fn_t)(
        nar_runtime_t rt,
        nar_object_t result,
        nar_cmd_state_t *cmd_state);

typedef void (*nar_program_cmd_exec_fn_t)(
        nar_runtime_t rt,
        void *user_data,
        nar_program_cmd_resolve_fn_t resolve,
        nar_cmd_state_t *cmd_state);

typedef nar_object_t (*nar_program_cmd_new_fn_t)(
        nar_runtime_t rt,
        nar_object_t to_msg,
        nar_program_cmd_exec_fn_t cmd_exec,
        void *user_data);

typedef void (*nar_program_sub_off_fn_t)(
        nar_runtime_t rt,
        void *user_data);

typedef struct {
    nar_serialized_object_t to_msg_list;
    nar_program_sub_off_fn_t off;
    void *user_data;
} nar_sub_state_t;

typedef void (*nar_program_sub_trigger_fn_t)(
        nar_runtime_t rt,
        nar_object_t value,
        nar_sub_state_t *sub_state);

typedef void (*nar_program_sub_on_fn_t)(
        nar_runtime_t rt,
        void *user_data,
        nar_program_sub_trigger_fn_t trigger,
        nar_sub_state_t *sub_state);

typedef nar_object_t (*nar_program_sub_new_fn_t)(
        nar_runtime_t rt,
        nar_object_t to_msg,
        nar_program_sub_on_fn_t on,
        nar_program_sub_off_fn_t off,
        void *user_data);

typedef void (*nar_program_flush_fn_t)(
        nar_runtime_t rt);

#endif //NAR_PROGRAM_H
