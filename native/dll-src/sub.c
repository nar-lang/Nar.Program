#include "_package.h"

nar_object_t meta__sub_new(
        nar_runtime_t rt, nar_object_t to_msg, nar_object_t payload,
        nar_program_sub_on_fn_t on, nar_program_sub_off_fn_t off) {
    return nar->make_tuple(rt, 4, (nar_object_t[]) {
            to_msg, payload, nar->make_native(rt, on, NULL), nar->make_native(rt, off, NULL)});
}

nar_object_t native__Sub_map(nar_runtime_t rt, nar_object_t f, nar_object_t subs) {
    nar_object_t it = subs;

    vector_t *v = nvector_new(sizeof(nar_object_t), 0, nar);

    while (nar->index_is_valid(rt, it)) {
        nar_list_item_t item = nar->to_list_item(rt, it);
        nar_tuple_t sub = nar->to_tuple(rt, item.value);
        nar_object_t mapped_sub = nar->make_tuple(rt, 4, (nar_object_t[]) {
                nar->make_list_cons(rt, f, sub.values[0]),
                sub.values[1], sub.values[2], sub.values[3]});
        vector_push(v, 1, &mapped_sub);
        it = item.next;
    }

    nar_object_t list = nar->make_list(rt, v->size, v->data);
    vector_free(v);
    return list;
}
