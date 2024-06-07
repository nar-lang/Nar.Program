#include "_package.h"

nar_object_t native__application(nar_runtime_t rt, nar_object_t app) { return app; }

void exit_exec(
        nar_runtime_t rt, nar_object_t payload, nar_program_cmd_resolve_fn_t resolve,
        nar_cmd_state_t *cmd_data) {
    state_t *state = nar->get_metadata(rt, NAR_META__Nar_Program__state);
    state->alive = false;
    state->exit_code = (int) nar->to_int(rt, payload);
    resolve(rt, NAR_INVALID_OBJECT, cmd_data);
}

nar_object_t native__exit(nar_runtime_t rt, nar_object_t exit_code) {
    return cmd_new(rt, NAR_INVALID_OBJECT, exit_code, &exit_exec);
}

nar_object_t native__args(nar_runtime_t rt) {
    args_t *args = nar->get_metadata(rt, NAR_META__Nar_Program__args);
    if (args == NULL) {
        return nar->make_list(rt, 0, NULL);
    }
    nar_object_t list = nar->make_list(rt, 0, NULL);
    for (size_t i = args->argc; i > 0; i--) {
        list = nar->make_list_cons(rt, nar->make_string(rt, args->argv[i - 1]), list);
    }
    return list;
}
