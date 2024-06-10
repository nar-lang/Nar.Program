#include "_package.h"

nar_object_t meta__task_new(
        nar_runtime_t rt, nar_object_t payload, nar_program_task_exec_fn_t task_exec) {
    return nar->make_tuple(rt, 2,
            (nar_object_t[]) {payload, nar->make_native(rt, task_exec, NULL)});
}

void succeed(
        nar_runtime_t rt,
        nar_object_t payload,
        nar_program_task_resolve_fn_t resolve,
        nar_program_task_reject_fn_t reject,
        nar_task_state_t *task_state) {
    resolve(rt, payload, task_state);
}

nar_object_t native__Task_succeed(nar_runtime_t rt, nar_object_t result) {
    return meta__task_new(rt, result, &succeed);
}

void fail(
        nar_runtime_t rt,
        nar_object_t payload,
        nar_program_task_resolve_fn_t resolve,
        nar_program_task_reject_fn_t reject,
        nar_task_state_t *task_state) {
    reject(rt, payload, task_state);
}

nar_object_t native__Task_fail(nar_runtime_t rt, nar_object_t error) {
    return meta__task_new(rt, error, &fail);
}

void attempt_resolve(nar_runtime_t rt, nar_object_t result, nar_task_state_t *task_state) {
    nar_object_t data = nar->make_option(rt, "Nar.Base.Result.Result#Ok", 1, &result);
    task_state->complete(rt, data, task_state->complete_user_data);
    nar->free(task_state);
}

void attempt_reject(nar_runtime_t rt, nar_object_t error, nar_task_state_t *task_state) {
    nar_object_t data = nar->make_option(rt, "Nar.Base.Result.Result#Err", 1, &error);
    task_state->complete(rt, data, task_state->complete_user_data);
    nar->free(task_state);
}

void attempt(
        nar_runtime_t rt, nar_object_t payload, nar_program_cmd_resolve_fn_t resolve,
        nar_cmd_state_t *cmd_state) {
    nar_tuple_t task = nar->to_tuple(rt, payload);
    nar_task_state_t *task_state = nar->alloc(sizeof(nar_task_state_t));
    task_state->complete = (nar_task_state_complete_fn_t) resolve;
    task_state->complete_user_data = cmd_state;
    task_state->user_data = NULL;
    nar_program_task_exec_fn_t exec = nar->to_native(rt, task.values[1]).ptr;
    exec(rt, task.values[0], &attempt_resolve, &attempt_reject, task_state);
}

typedef struct {
    nar_object_t next_task_closure;
    nar_program_task_resolve_fn_t resolve;
    nar_program_task_reject_fn_t reject;
    void *prev_user_data;
} next_task_state_t;

void chain_next(nar_runtime_t rt, nar_object_t result, nar_task_state_t *task_state) {
    next_task_state_t *next_state = task_state->user_data;
    task_state->user_data = next_state->prev_user_data;
    nar_object_t next_task_obj = nar->apply_func(rt, next_state->next_task_closure, 1, &result);
    nar->free(next_state);

    nar_tuple_t next_task = nar->to_tuple(rt, next_task_obj);
    nar_program_task_exec_fn_t exec = nar->to_native(rt, next_task.values[1]).ptr;
    exec(rt, next_task.values[0], &attempt_resolve, &attempt_reject, task_state);
}

void chain(
        nar_runtime_t rt,
        nar_object_t payload,
        nar_program_task_resolve_fn_t resolve,
        nar_program_task_reject_fn_t reject,
        nar_task_state_t *task_state) {
    nar_tuple_t chain_data = nar->to_tuple(rt, payload);
    nar_object_t task_a_object = chain_data.values[0];
    nar_object_t task_b_closure = chain_data.values[1];
    nar_tuple_t task_a = nar->to_tuple(rt, task_a_object);

    next_task_state_t *next_task_state = nar->alloc(sizeof(next_task_state_t));
    next_task_state->next_task_closure = task_b_closure;
    next_task_state->prev_user_data = task_state->user_data;
    next_task_state->resolve = resolve;
    next_task_state->reject = reject;
    task_state->user_data = next_task_state;

    nar_program_task_exec_fn_t exec = nar->to_native(rt, task_a.values[1]).ptr;
    if (nar->to_bool(rt, chain_data.values[2])) {
        exec(rt, task_a.values[0], &chain_next, reject, task_state);
    } else {
        exec(rt, task_a.values[0], resolve, &chain_next, task_state);
    }
}

nar_object_t native__Task_attempt(nar_runtime_t rt, nar_object_t toMessage, nar_object_t task) {
    return meta__cmd_new(rt, toMessage, task, &attempt);
}

nar_object_t native__Task_onError(nar_runtime_t rt, nar_object_t nextTask, nar_object_t task) {
    nar_object_t task_objects[3] = {task, nextTask, nar->make_bool(rt, false)};
    nar_object_t chain_data = nar->make_tuple(rt, 3, task_objects);
    return meta__task_new(rt, chain_data, &chain);
}

nar_object_t native__Task_andThen(nar_runtime_t rt, nar_object_t nextTask, nar_object_t task) {
    nar_object_t task_objects[3] = {task, nextTask, nar->make_bool(rt, true)};
    nar_object_t chain_data = nar->make_tuple(rt, 3, task_objects);
    return meta__task_new(rt, chain_data, &chain);
}
