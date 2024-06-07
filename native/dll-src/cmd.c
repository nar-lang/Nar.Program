#include "_package.h"

nar_object_t cmd_new(
        nar_runtime_t rt,
        nar_object_t to_msg,
        nar_object_t payload,
        nar_program_cmd_exec_fn_t cmd_exec) {
    nar_object_t to_msg_list = nar->make_list(rt, 1, &to_msg);
    nar_object_t cmd_exec_obj = nar->make_native(rt, cmd_exec, NULL);
    return nar->make_tuple(rt, 3, (nar_object_t[]) {to_msg_list, payload, cmd_exec_obj});
}

nar_object_t native__Cmd_map(nar_runtime_t rt, nar_object_t f, nar_object_t cmds) {
    nar_object_t it = cmds;
    vector_t *v = nvector_new(sizeof(nar_object_t), 0, nar);

    while (nar->index_is_valid(rt, it)) {
        nar_list_item_t item = nar->to_list_item(rt, it);
        nar_tuple_t cmd = nar->to_tuple(rt, item.value);
        nar_object_t to_msg_list = nar->make_list_cons(rt, f, cmd.values[0]);
        nar_object_t new_cmd = nar->make_tuple(rt, 3,
                (nar_object_t[]) {to_msg_list, cmd.values[1], cmd.values[2]});
        vector_push(v, 1, &new_cmd);
        it = item.next;
    }

    nar_object_t list = nar->make_list(rt, v->size, v->data);
    vector_free(v);
    return list;
}
