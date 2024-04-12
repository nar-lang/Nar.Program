#include "_package.h"

nar_object_t cmd_new(nar_runtime_t rt, cmd_exec_fn_t exec, nar_object_t to_msg, nar_object_t arg) {
    return nar->make_tuple(rt, 3,
            (nar_object_t[]) {
                    nar->make_native(rt, exec, NULL),
                    nar->make_list(rt, 1, &to_msg),
                    arg,
            });
}

void cmd_pass_exec(
        nar_runtime_t rt, nar_object_t data, cmd_callback_fn_t callback, void *cmd_state) {
    callback(rt, data, cmd_state);
}

nar_object_t cmd_pass(nar_runtime_t rt, nar_object_t msg) {
    return cmd_new(rt, cmd_pass_exec, msg, NAR_INVALID_OBJECT);
}

nar_object_t cmd_passValue(nar_runtime_t rt, nar_object_t to_msg, nar_object_t value) {
    return cmd_new(rt, cmd_pass_exec, to_msg, value);
}

nar_object_t cmd_map(nar_runtime_t rt, nar_object_t f, nar_object_t cmds) {
    if (!nar->index_is_valid(rt, cmds)) {
        return nar->make_list(rt, 0, NULL);
    }
    nar_list_item_t cmd_item = nar->to_list_item(rt, cmds);
    nar_tuple_item_t cmd_exec = nar->to_tuple_item(rt, cmd_item.value);
    nar_tuple_item_t cmd_to_msg = nar->to_tuple_item(rt, cmd_item.next);
    nar_tuple_item_t cmd_arg = nar->to_tuple_item(rt, cmd_to_msg.next);

    return nar->make_list_cons(rt,
            nar->make_tuple(rt, 3,
                    (nar_object_t[]) {
                            cmd_exec.value,
                            nar->make_list_cons(rt, f, cmd_to_msg.value),
                            cmd_arg.value,
                    }),
            cmd_map(rt, f, cmd_item.next));
}
