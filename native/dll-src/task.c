#include "_package.h"

nar_object_t task_new(
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
    return task_new(rt, result, &succeed);
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
    return task_new(rt, error, &fail);
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
    nar_program_task_exec_fn_t exec;
    nar_serialized_object_t payload;
    nar_program_task_resolve_fn_t resolve;
    nar_program_task_reject_fn_t reject;
    void *prev_user_data;
} next_task_state_t;

void chain_next(nar_runtime_t rt, nar_object_t result, nar_task_state_t *task_state) {
    next_task_state_t *next_state = task_state->user_data;
    task_state->user_data = next_state->prev_user_data;
    nar_object_t payload = nar->deserialize_object(rt, next_state->payload);
    nar->free(next_state->payload);
    next_state->exec(rt, payload, next_state->resolve, next_state->reject, task_state);
    nar->free(next_state);
}

void chain(
        nar_runtime_t rt,
        nar_object_t payload,
        nar_program_task_resolve_fn_t resolve,
        nar_program_task_reject_fn_t reject,
        nar_task_state_t *task_state) {
    nar_tuple_t chain_data = nar->to_tuple(rt, payload);
    nar_object_t task_a_object = chain_data.values[0];
    nar_object_t task_b_object = chain_data.values[1];
    nar_tuple_t task_a = nar->to_tuple(rt, task_a_object);
    nar_tuple_t task_b = nar->to_tuple(rt, task_b_object);

    next_task_state_t *next_task_state = nar->alloc(sizeof(next_task_state_t));
    next_task_state->exec = nar->to_native(rt, task_b.values[1]).ptr;
    next_task_state->payload = nar->new_serialized_object(rt, task_b.values[0]);
    next_task_state->prev_user_data = task_state->user_data;
    next_task_state->resolve = resolve;
    next_task_state->reject = reject;
    task_state->user_data = next_task_state;

    nar_program_task_exec_fn_t exec = nar->to_native(rt, task_a.values[1]).ptr;
    if (chain_data.values[2] == nar->make_bool(rt, true)) {
        exec(rt, task_a.values[0], &chain_next, reject, task_state);
    } else {
        exec(rt, task_a.values[0], resolve, &chain_next, task_state);
    }
}

nar_object_t native__Task_andThen(nar_runtime_t rt, nar_object_t nextTask, nar_object_t task) {
    nar_object_t task_objects[3] = {task, nextTask, nar->make_bool(rt, true)};
    nar_object_t chain_data = nar->make_tuple(rt, 2, task_objects);
    return task_new(rt, chain_data, &chain);
}

nar_object_t native__Task_onError(nar_runtime_t rt, nar_object_t nextTask, nar_object_t task) {
    nar_object_t task_objects[3] = {task, nextTask, nar->make_bool(rt, false)};
    nar_object_t chain_data = nar->make_tuple(rt, 2, task_objects);
    return task_new(rt, chain_data, &chain);
}

nar_object_t native__Task_attempt(
        nar_runtime_t rt, nar_object_t resultToMessage, nar_object_t task) {
    return cmd_new(rt, resultToMessage, task, &attempt);
}
