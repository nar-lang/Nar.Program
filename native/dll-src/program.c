#include "_package.h"

nar_object_t program_application(nar_runtime_t rt, nar_object_t app) {
    return app;
}

void *to_serialized_closure(nar_runtime_t rt, nar_object_t program, nar_cstring_t callbackName) {
    nar_object_t field = nar->to_record_field(rt, program, callbackName);
    if (nar->object_is_valid(rt, field)) {
        return nar->new_serialized_object(rt, field);
    }
    return NULL;
}

void enqueue_message(nar_runtime_t rt, nar_object_t msg) {
    state_t *state = nar->get_metadata(rt, "Nar.Program:state");
    void *serialized_msg = nar->new_serialized_object(rt, msg);
    vector_push(state->msgs, 1, &serialized_msg);
}

void enqueue_message_data(nar_runtime_t rt, nar_object_t to_msg_arg, nar_object_t to_msg_chain) {
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

void command_callback(nar_runtime_t rt, nar_object_t to_msg_arg, void *serialized_to_msg) {
    nar_object_t to_msg_chain = nar->deserialize_object(rt, serialized_to_msg);
    enqueue_message_data(rt, to_msg_arg, to_msg_chain);
    nar->free(serialized_to_msg);
}

void enqueue_commands(nar_runtime_t rt, nar_object_t cmds) {
    state_t *state = nar->get_metadata(rt, "Nar.Program:state");
    nar_object_t it = cmds;
    while (nar->index_is_valid(rt, it)) {
        nar_list_item_t item = nar->to_list_item(rt, it);
        void *serialized_cmd = nar->new_serialized_object(rt, item.value);
        vector_push(state->cmds, 1, &serialized_cmd);
        it = item.next;
    }
}

void process_command(nar_runtime_t rt, void *serialized_cmd) {
    nar_object_t cmd = nar->deserialize_object(rt, serialized_cmd);
    nar_tuple_item_t cmd_exec = nar->to_tuple_item(rt, cmd);
    nar_tuple_item_t cmd_to_msg = nar->to_tuple_item(rt, cmd_exec.next);
    nar_tuple_item_t cmd_arg = nar->to_tuple_item(rt, cmd_to_msg.next);
    cmd_exec_fn_t exec = (cmd_exec_fn_t) nar->to_native(rt, cmd_exec.value).ptr;
    exec(rt, cmd_arg.value, command_callback, nar->new_serialized_object(rt, cmd_to_msg.value));
}

void process_commands(nar_runtime_t rt, state_t *state) {
    for (size_t i = 0; i < vector_size(state->cmds); i++) {
        void *serialized_cmd = *(void **) vector_at(state->cmds, i);
        process_command(rt, serialized_cmd);
        nar->free(serialized_cmd);
        if (!state->alive) {
            for (size_t j = i + 1; j < vector_size(state->cmds); j++) {
                nar->free(*(void **) vector_at(state->cmds, j));
            }
            for (size_t j = 0; j < vector_size(state->msgs); j++) {
                nar->free(*(void **) vector_at(state->msgs, j));
            }
            vector_clear(state->msgs);
            break;
        }
    }
    vector_clear(state->cmds);
}

void apply_model_and_cmds(nar_runtime_t rt, state_t *state, nar_object_t model_and_cmds) {
    nar_tuple_item_t model_item = nar->to_tuple_item(rt, model_and_cmds);
    nar->free(state->model);
    state->model = nar->new_serialized_object(rt, model_item.value);
    nar_tuple_item_t cmds_item = nar->to_tuple_item(rt, model_item.next);
    enqueue_commands(rt, cmds_item.value);
}

void process_message(nar_runtime_t rt, state_t *state, void *serialized_msg) {
    nar->frame_free(rt);
    nar_object_t model = nar->deserialize_object(rt, state->model);
    nar_object_t msg = nar->deserialize_object(rt, serialized_msg);
    nar_object_t update = nar->deserialize_object(rt, state->update);
    nar_object_t model_and_cmds = nar->apply_func(rt, update, 2, (nar_object_t[2]) {msg, model});
    apply_model_and_cmds(rt, state, model_and_cmds);
}

bool process_messages(
        nar_runtime_t rt, state_t *state) {
    for (size_t i = 0; i < vector_size(state->msgs); i++) {
        void *serialized_msg = *(void **) vector_at(state->msgs, i);
        process_message(rt, state, serialized_msg);
        nar->free(serialized_msg);
    }
    bool processed = vector_size(state->msgs) > 0;
    vector_clear(state->msgs);
    return processed;
}

void state_free(nar_runtime_t rt) {
    //TODO: unsubscribe all
    state_t *state = nar->get_metadata(rt, "Nar.Program:state");
    if (state != NULL) {
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
        nar->free(state);
        nar->set_metadata(rt, "Nar.Program:state", NULL);
    }
}

void step(nar_runtime_t rt) {
    state_t *state = nar->get_metadata(rt, "Nar.Program:state");
    bool model_changed = false;
    while (vector_size(state->cmds) > 0) {
        process_commands(rt, state);
        model_changed = process_messages(rt, state);
    }

    if (state->alive && model_changed) {
        nar->frame_free(rt);
        nar_object_t model = nar->deserialize_object(rt, state->model);

        nar_object_t view = nar->deserialize_object(rt, state->view);
        nar_object_t scene = nar->apply_func(rt, view, 1, &model);

        nar_object_t subscribe = nar->deserialize_object(rt, state->subscribe);
        nar_object_t subs = nar->apply_func(rt, subscribe, 1, &model);

        nar_object_t present = nar->deserialize_object(rt, state->present);
        nar->apply_func(rt, present, 1, &scene);
    }

    if (!state->alive) {
        exit_callback_fn_t exit_callback = state->exit_callback;
        int exit_code = state->exit_code;
        state_free(rt);
        if (exit_callback) {
            exit_callback(rt, exit_code);
        }
    }
}

void program_exit_exec(
        nar_runtime_t rt, nar_object_t data, cmd_callback_fn_t callback, void *cmd_state) {
    state_t *state = nar->get_metadata(rt, "Nar.Program:state");
    state->alive = false;
    state->exit_code = (int) nar->to_int(rt, data);

    callback(rt, data, cmd_state);
}

nar_object_t program_exit(nar_runtime_t rt, nar_object_t exit_code) {
    return cmd_new(rt, program_exit_exec, NAR_INVALID_OBJECT, exit_code);
}

void program_execute(nar_runtime_t rt, nar_object_t program, exit_callback_fn_t exit) {

    if (nar->object_get_kind(rt, program) != NAR_OBJECT_KIND_OPTION) {
        return;
    }
    nar_option_t program_option = nar->to_option(rt, program);
    if (strcmp(program_option.name, "Nar.Program.Program#Program") != 0) {
        return;
    }
    if (program_option.size != 1) {
        return;
    }

    nar_object_t lifecycle = program_option.values[0];

    state_t *state = nar->alloc(sizeof(state_t));
    state->alive = true;
    state->exit_code = 0;
    state->cmds = nvector_new(sizeof(void *), 0, nar);
    state->msgs = nvector_new(sizeof(void *), 0, nar);
    state->update = to_serialized_closure(rt, lifecycle, "update");
    state->subscribe = to_serialized_closure(rt, lifecycle, "subscribe");
    state->view = to_serialized_closure(rt, lifecycle, "view");
    state->present = to_serialized_closure(rt, lifecycle, "present");
    state->model = NULL;
    state->exit_callback = exit;
    nar->set_metadata(rt, "Nar.Program:state", state);

    nar_object_t flags = nar->to_record_field(rt, lifecycle, "flags");
    if (!nar->object_is_valid(rt, flags)) {
        flags = nar->make_list(rt, 0, NULL);
    }

    nar_object_t init_fn = nar->to_record_field(rt, lifecycle, "init");
    if (!nar->index_is_valid(rt, init_fn)) {
        nar->fail(rt, "Program does not have an init field");
        return;
    }

    nar_object_t model_and_cmds = nar->apply_func(rt, init_fn, 1, &flags);
    apply_model_and_cmds(rt, state, model_and_cmds);

    step(rt);
}
