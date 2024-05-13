#include "_package.h"

nar_object_t cmd_new(
        nar_runtime_t rt,
        nar_object_t to_msg,
        nar_program_cmd_exec_fn_t cmd_exec,
        void *user_data) {
    cmd_t *cmd = nar->frame_alloc(rt, sizeof(cmd_t));
    cmd->exec = cmd_exec;
    cmd->user_data = user_data;
    cmd->to_msg_list = nar->make_list(rt, 1, &to_msg);
    return nar->make_native(rt, cmd, NULL);
}

nar_object_t cmd_map(nar_runtime_t rt, nar_object_t f, nar_object_t cmds) {
    nar_object_t it = cmds;

    vector_t *v = nvector_new(sizeof(nar_object_t), 0, nar);

    while (nar->index_is_valid(rt, it)) {
        nar_list_item_t item = nar->to_list_item(rt, it);
        cmd_t *cmd = nar->to_native(rt, item.value).ptr;
        cmd_t *new_cmd = nar->frame_alloc(rt, sizeof(cmd_t));
        *new_cmd = *cmd;
        new_cmd->to_msg_list = nar->make_list_cons(rt, f, cmd->to_msg_list);
        nar_object_t new_cmd_obj = nar->make_native(rt, new_cmd, NULL);
        vector_push(v, 1, &new_cmd_obj);
        it = item.next;
    }

    nar_object_t list = nar->make_list(rt, v->size, v->data);
    vector_free(v);
    return list;
}
