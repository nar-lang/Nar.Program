#include "_package.h"

void unsubscribe(nar_runtime_t rt, state_t *state) {
    for (size_t i = 0; i < vector_size(state->subs); i++) {
        nar_sub_state_t *s = *(nar_sub_state_t **) vector_at(state->subs, i);
        s->off(rt, nar->deserialize_object(rt, s->payload));
        nar->free(s->payload);
        nar->free(s->to_msg_list);
        nar->free(s);
    }
    vector_clear(state->subs);
}

void state_free(nar_runtime_t rt) {
    state_t *state = nar->get_metadata(rt, NAR_META__Nar_Program__state);
    if (state != NULL) {
        unsubscribe(rt, state);
        for (size_t i = 0; i < vector_size(state->cmds); i++) {
            nar->free(*(void **) vector_at(state->cmds, i));
        }
        vector_free(state->cmds);
        for (size_t i = 0; i < vector_size(state->msgs); i++) {
            nar->free(*(void **) vector_at(state->msgs, i));
        }
        vector_free(state->msgs);
        nar->free(state->update);
        nar->free(state->subscribe);
        nar->free(state->view);
        nar->free(state->present);
        nar->free(state->model);
        nar->free(state->subs);
        nar->free(state);
        nar->set_metadata(rt, NAR_META__Nar_Program__state, NULL);
    }
}

void enqueue_message(nar_runtime_t rt, nar_object_t msg) {
    state_t *state = nar->get_metadata(rt, NAR_META__Nar_Program__state);
    void *serialized_msg = nar->new_serialized_object(rt, msg);
    vector_push(state->msgs, 1, &serialized_msg);
}

void enqueue_side_effect(nar_runtime_t rt, nar_object_t to_msg_arg, nar_object_t to_msg_chain) {
    nar_object_t it = to_msg_chain;
    nar_object_t result = to_msg_arg;
    while (nar->index_is_valid(rt, it)) {
        nar_list_item_t item = nar->to_list_item(rt, it);
        if (nar->object_is_valid(rt, result) && nar->object_is_valid(rt, item.value)) {
            result = nar->apply_func(rt, item.value, 1, &result);
        } else {
            result = item.value;
        }
        it = item.next;
    }
    enqueue_message(rt, result);
}

void trigger_sub(nar_runtime_t rt, nar_object_t value, nar_sub_state_t *sub_state) {
    nar_object_t to_msg_list = nar->deserialize_object(rt, sub_state->to_msg_list);
    state_t *state = nar->get_metadata(rt, NAR_META__Nar_Program__state);
    for (size_t i = 0; i < vector_size(state->subs); i++) {
        nar_sub_state_t *ss = *(nar_sub_state_t **) vector_at(state->subs, i);
        if (ss == sub_state) {
            enqueue_side_effect(rt, value, to_msg_list);
            return;
        }
    }
    nar->fail(rt, "sub triggered which were not subscribed to");
}

void process_sub(nar_runtime_t rt, state_t *state, nar_object_t sub_object) {
    sub_t *sub = nar->to_native(rt, sub_object).ptr;
    nar_sub_state_t *sub_state = nar->alloc(sizeof(nar_sub_state_t));
    sub_state->to_msg_list = nar->new_serialized_object(rt, sub->to_msg_list);
    sub_state->off = sub->off;
    sub_state->payload = nar->new_serialized_object(rt, sub->payload);
    sub->on(rt, sub->payload, trigger_sub, sub_state);
    vector_push(state->subs, 1, &sub_state);
}

void process_subs(nar_runtime_t rt, state_t *state, nar_object_t subs) {
    unsubscribe(rt, state);
    nar_object_t it = subs;
    while (nar->index_is_valid(rt, it)) {
        nar_list_item_t item = nar->to_list_item(rt, it);
        process_sub(rt, state, item.value);
        it = item.next;
    }
}

void process_model_changed(nar_runtime_t rt, state_t *state) {
    if (state->alive) {
        nar->frame_free(rt);
        nar_object_t model = nar->deserialize_object(rt, state->model);

        nar_object_t view = nar->deserialize_object(rt, state->view);
        nar_object_t scene = nar->apply_func(rt, view, 1, &model);

        nar_object_t subscribe = nar->deserialize_object(rt, state->subscribe);
        nar_object_t subs = nar->apply_func(rt, subscribe, 1, &model);
        process_subs(rt, state, subs);

        nar_object_t present = nar->deserialize_object(rt, state->present);
        nar->apply_func(rt, present, 1, &scene);
    }
}

int enqueue_commands(nar_runtime_t rt, nar_object_t cmds) {
    state_t *state = nar->get_metadata(rt, NAR_META__Nar_Program__state);
    nar_object_t it = cmds;
    int num_commands = 0;
    while (nar->index_is_valid(rt, it)) {
        nar_list_item_t item = nar->to_list_item(rt, it);
        void *serialized_cmd = nar->new_serialized_object(rt, item.value);
        vector_push(state->cmds, 1, &serialized_cmd);
        it = item.next;
        num_commands++;
    }
    return num_commands;
}

int apply_model_and_cmds(nar_runtime_t rt, state_t *state, nar_object_t model_and_cmds) {
    nar_tuple_item_t model_item = nar->to_tuple_item(rt, model_and_cmds);
    nar->free(state->model);
    state->model = nar->new_serialized_object(rt, model_item.value);
    nar_tuple_item_t cmds_item = nar->to_tuple_item(rt, model_item.next);
    return enqueue_commands(rt, cmds_item.value);
}

void process_message(nar_runtime_t rt, state_t *state, void *serialized_msg) {
    nar->frame_free(rt);
    nar_object_t model = nar->deserialize_object(rt, state->model);
    nar_object_t msg = nar->deserialize_object(rt, serialized_msg);
    nar_object_t update = nar->deserialize_object(rt, state->update);
    nar_object_t model_and_cmds = nar->apply_func(rt, update, 2, (nar_object_t[2]) {msg, model});
    apply_model_and_cmds(rt, state, model_and_cmds);
}

bool process_messages(nar_runtime_t rt, state_t *state) {
    for (size_t i = 0; i < vector_size(state->msgs); i++) {
        void *serialized_msg = *(void **) vector_at(state->msgs, i);
        process_message(rt, state, serialized_msg);
        nar->free(serialized_msg);
    }
    bool processed = vector_size(state->msgs) > 0;
    vector_clear(state->msgs);
    return processed;
}

void resolve_command(nar_runtime_t rt, nar_object_t result, nar_cmd_state_t *cmd_state) {
    nar_object_t cmd_object = nar->deserialize_object(rt, cmd_state->serialized_cmd);
    nar_tuple_t cmd = nar->to_tuple(rt, cmd_object);
    nar_object_t to_msg_list = cmd.values[0];
    enqueue_side_effect(rt, result, to_msg_list);
    nar->free(cmd_state->serialized_cmd);
    nar->free(cmd_state);
}

void process_command(nar_runtime_t rt, nar_serialized_object_t serialized_cmd) {
    nar_object_t cmd_object = nar->deserialize_object(rt, serialized_cmd);
    nar_tuple_t cmd = nar->to_tuple(rt, cmd_object);
    nar_cmd_state_t *state = nar->alloc(sizeof(nar_cmd_state_t));
    state->serialized_cmd = serialized_cmd;
    nar_object_t payload = cmd.values[1];
    nar_program_cmd_exec_fn_t exec = nar->to_native(rt, cmd.values[2]).ptr;
    exec(rt, payload, resolve_command, state);
}

void process_commands(nar_runtime_t rt, state_t *state) {
    for (size_t i = 0; i < vector_size(state->cmds); i++) {
        nar_serialized_object_t serialized_cmd = *(nar_serialized_object_t *) vector_at(
                state->cmds, i);
        process_command(rt, serialized_cmd);
        if (!state->alive) {
            for (size_t j = i + 1; j < vector_size(state->cmds); j++) {
                nar->free(*(nar_serialized_object_t *) vector_at(state->cmds, j));
            }
            for (size_t j = 0; j < vector_size(state->msgs); j++) {
                nar->free(*(nar_serialized_object_t *) vector_at(state->msgs, j));
            }
            vector_clear(state->msgs);
            break;
        }
    }
    vector_clear(state->cmds);
}

void step(nar_runtime_t rt, state_t *state) {
    bool model_changed = false;
    bool force = true;
    while (force || vector_size(state->cmds) > 0) {
        force = false;
        process_commands(rt, state);
        model_changed |= process_messages(rt, state);
    }

    if (model_changed) {
        process_model_changed(rt, state);
    }

    if (!state->alive) {
        nar_program_exit_cb_t exit_callback = state->exit_callback;
        int exit_code = state->exit_code;
        state_free(rt);
        if (exit_callback != NULL) {
            exit_callback(rt, exit_code);
        }
    }
}

nar_serialized_object_t to_serialized_closure(
        nar_runtime_t rt, nar_object_t program, nar_cstring_t callbackName) {
    nar_object_t field = nar->to_record_field(rt, program, callbackName);
    if (nar->object_is_valid(rt, field)) {
        return nar->new_serialized_object(rt, field);
    }
    return NULL;
}

void set_args(nar_runtime_t rt, int argc, char **argv) {
    args_t *args = nar->get_metadata(rt, NAR_META__Nar_Program__args);
    if (args == NULL) {
        args = nar->alloc(sizeof(args_t));
    }
    args->argc = argc;
    args->argv = argv;
    nar->set_metadata(rt, NAR_META__Nar_Program__args, args);
}

int execute(
        nar_runtime_t rt,
        nar_object_t program,
        nar_program_main_update_cb_t update_cb,
        nar_program_exit_cb_t exit_cb) {
    if (nar->object_get_kind(rt, program) != NAR_OBJECT_KIND_OPTION) {
        return -1;
    }
    nar_option_t program_option = nar->to_option(rt, program);
    if (strcmp(program_option.name, Nar_Program_Program__Program) != 0) {
        return -2;
    }
    if (program_option.size != 1) {
        return -3;
    }

    nar_object_t lifecycle = program_option.values[0];

    state_t *state = nar->alloc(sizeof(state_t));
    state->alive = true;
    state->exit_code = 0;
    state->cmds = nvector_new(sizeof(nar_serialized_object_t), 0, nar);
    state->msgs = nvector_new(sizeof(nar_serialized_object_t), 0, nar);
    state->subs = nvector_new(sizeof(nar_sub_state_t *), 0, nar);
    state->update = to_serialized_closure(rt, lifecycle, "update");
    state->subscribe = to_serialized_closure(rt, lifecycle, "subscribe");
    state->view = to_serialized_closure(rt, lifecycle, "view");
    state->present = to_serialized_closure(rt, lifecycle, "present");
    state->model = NULL;
    state->exit_callback = exit_cb;
    nar->set_metadata(rt, NAR_META__Nar_Program__state, state);

    nar_object_t flags = nar->to_record_field(rt, lifecycle, "flags");
    if (!nar->object_is_valid(rt, flags)) {
        flags = nar->make_list(rt, 0, NULL);
    }

    nar_object_t init_fn = nar->to_record_field(rt, lifecycle, "init");
    if (!nar->index_is_valid(rt, init_fn)) {
        nar->fail(rt, "Program does not have an init field");
        return -4;
    }

    nar_object_t model_and_cmds = nar->apply_func(rt, init_fn, 1, &flags);
    if (0 == apply_model_and_cmds(rt, state, model_and_cmds)) {
        process_model_changed(rt, state);
    }

    step(rt, state);

    if (update_cb != NULL) {
        while (state->alive) {
            update_cb(rt);
            flush(rt);
        }
    }
    return state->exit_code;
}

nar_bool_t flush(nar_runtime_t rt) {
    state_t *state = nar->get_metadata(rt, NAR_META__Nar_Program__state);
    step(rt, state);
    return state->alive;
}
