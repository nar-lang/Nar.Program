#ifndef NAR_PROGRAM_H
#define NAR_PROGRAM_H

#include <nar.h>

#define Nar_Program_Program "Nar.Program.Program"
#define Nar_Program_Program__Program "Nar.Program.Program#Program"

#define NAR_META__Nar_Program_set_args "Nar.Program:set_args"
#define NAR_META__Nar_Program_execute "Nar.Program:execute"
#define NAR_META__Nar_Program_cmd_new "Nar.Program:cmd_new"
#define NAR_META__Nar_Program_sub_new "Nar.Program:sub_new"
#define NAR_META__Nar_Program_task_new "Nar.Program:task_new"
#define NAR_META__Nar_Program_update "Nar.Program:update"
#define NAR_META__Nar_Program_updater_add "Nar.Program:updater_add"
#define NAR_META__Nar_Program_updater_remove "Nar.Program:updater_remove"

typedef void (*nar_program_set_args_fn_t)(
        nar_runtime_t rt, int argc, char **argv);

typedef void (*nar_program_exit_cb_t)(
        nar_runtime_t rt,
        nar_int_t exit_code);

typedef int (*nar_program_execute_fn_t)(
        nar_runtime_t rt,
        nar_object_t program,
        nar_program_exit_cb_t exit);

typedef struct {
    nar_serialized_object_t serialized_cmd;
} nar_cmd_state_t;

typedef void (*nar_program_cmd_resolve_fn_t)(
        nar_runtime_t rt,
        nar_object_t result,
        nar_cmd_state_t *cmd_state);

typedef void (*nar_program_cmd_exec_fn_t)(
        nar_runtime_t rt,
        nar_object_t payload,
        nar_program_cmd_resolve_fn_t resolve,
        nar_cmd_state_t *cmd_state);

typedef nar_object_t (*nar_program_cmd_new_fn_t)(
        nar_runtime_t rt,
        nar_object_t to_msg,
        nar_object_t payload,
        nar_program_cmd_exec_fn_t cmd_exec);

typedef void (*nar_program_sub_off_fn_t)(
        nar_runtime_t rt,
        nar_object_t payload);

typedef struct {
    nar_serialized_object_t to_msg_list;
    nar_program_sub_off_fn_t off;
    nar_serialized_object_t payload;
} nar_sub_state_t;

typedef void (*nar_program_sub_trigger_fn_t)(
        nar_runtime_t rt,
        nar_object_t value,
        nar_sub_state_t *sub_state);

typedef void (*nar_program_sub_on_fn_t)(
        nar_runtime_t rt,
        nar_object_t payload,
        nar_program_sub_trigger_fn_t trigger,
        nar_sub_state_t *sub_state);

typedef nar_object_t (*nar_program_sub_new_fn_t)(
        nar_runtime_t rt,
        nar_object_t to_msg,
        nar_object_t payload,
        nar_program_sub_on_fn_t on,
        nar_program_sub_off_fn_t off);


typedef nar_bool_t (*nar_program_update_cb_fn_t)(
        nar_runtime_t rt);

typedef void (*nar_program_updater_add_t)(
        nar_runtime_t rt,
        nar_program_update_cb_fn_t update,
        nar_int_t priority);

typedef void (*nar_program_updater_remove_t)(
        nar_runtime_t rt,
        nar_program_update_cb_fn_t update);

typedef void (*nar_task_state_complete_fn_t)(
        nar_runtime_t rt,
        nar_object_t result,
        void *user_data);

typedef struct {
    nar_task_state_complete_fn_t complete;
    void *complete_user_data;
    void *user_data;
} nar_task_state_t;

typedef void (*nar_program_task_resolve_fn_t)(
        nar_runtime_t rt,
        nar_object_t result,
        nar_task_state_t *task_state);

typedef void (*nar_program_task_reject_fn_t)(
        nar_runtime_t rt,
        nar_object_t error,
        nar_task_state_t *task_state);

typedef void (*nar_program_task_exec_fn_t)(
        nar_runtime_t rt,
        nar_object_t payload,
        nar_program_task_resolve_fn_t resolve,
        nar_program_task_reject_fn_t reject,
        nar_task_state_t *task_state);

typedef nar_object_t (*nar_program_task_new_fn_t)(
        nar_runtime_t rt,
        nar_object_t payload,
        nar_program_task_exec_fn_t task_exec);

#endif //NAR_PROGRAM_H
