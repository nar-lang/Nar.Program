#include "_package.h"

nar_object_t sub_new(
        nar_runtime_t rt, nar_object_t to_msg, nar_object_t payload,
        nar_program_sub_on_fn_t on, nar_program_sub_off_fn_t off) {
    sub_t *sub = nar->frame_alloc(rt, sizeof(sub_t));
    sub->on = on;
    sub->off = off;
    sub->payload = payload;
    sub->to_msg_list = nar->make_list(rt, 1, &to_msg);
    return nar->make_native(rt, sub, NULL);
}

nar_object_t native__Sub_map(nar_runtime_t rt, nar_object_t f, nar_object_t subs) {
    nar_object_t it = subs;

    vector_t *v = nvector_new(sizeof(nar_object_t), 0, nar);

    while (nar->index_is_valid(rt, it)) {
        nar_list_item_t item = nar->to_list_item(rt, it);
        sub_t *sub = nar->to_native(rt, item.value).ptr;
        sub_t *new_sub = nar->frame_alloc(rt, sizeof(sub_t));
        *new_sub = *sub;
        new_sub->to_msg_list = nar->make_list_cons(rt, f, sub->to_msg_list);
        nar_object_t new_sub_obj = nar->make_native(rt, new_sub, NULL);
        vector_push(v, 1, &new_sub_obj);
        it = item.next;
    }

    nar_object_t list = nar->make_list(rt, v->size, v->data);
    vector_free(v);
    return list;
}
