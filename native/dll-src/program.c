#include "_package.h"

nar_object_t program_application(nar_runtime_t rt, nar_object_t app) { return app; }

void program_exit_exec(
        nar_runtime_t rt, void *user_data, nar_program_cmd_resolve_fn_t resolve,
        nar_cmd_state_t *cmd_data) {
    state_t *state = nar->get_metadata(rt, NAR_METADATA_NAR_PROGRAM_STATE);
    state->alive = false;
    nar_object_t exit_code = nar->deserialize_object(rt, user_data);
    state->exit_code = (int) nar->to_int(rt, exit_code);
    nar->free(user_data);
    resolve(rt, NAR_INVALID_OBJECT, cmd_data);
}

nar_object_t program_exit(nar_runtime_t rt, nar_object_t exit_code) {
    nar_serialized_object_t user_data = nar->new_serialized_object(rt, exit_code);
    return cmd_new(rt, NAR_INVALID_OBJECT, program_exit_exec, user_data);
}
