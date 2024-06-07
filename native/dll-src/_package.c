#include "_package.h"

nar_t *nar;

nar_int_t init(nar_t *n, nar_runtime_t rt) {
    nar = n;
    nar->register_def_dynamic(rt, "Nar.Program.Cmd", "map", "native__Cmd_map", 2);
    nar->register_def_dynamic(rt, "Nar.Program", "application", "native__application", 1);
    nar->register_def_dynamic(rt, "Nar.Program", "args", "native__args", 0 );
    nar->register_def_dynamic(rt, "Nar.Program", "exit", "native__exit", 1);
    nar->register_def_dynamic(rt, "Nar.Program.Sub", "map", "native__Sub_map", 2);
    nar->register_def_dynamic(rt, "Nar.Program.Task", "succeed", "native__Task_succeed", 1);
    nar->register_def_dynamic(rt, "Nar.Program.Task", "fail", "native__Task_fail", 1);
    nar->register_def_dynamic(rt, "Nar.Program.Task", "andThen", "native__Task_andThen", 2);
    nar->register_def_dynamic(rt, "Nar.Program.Task", "onError", "native__Task_onError", 2);
    nar->register_def_dynamic(rt, "Nar.Program.Task", "attempt", "native__Task_attempt", 2);

    nar->set_metadata(rt, NAR_META__Nar_Program_set_args, &set_args);
    nar->set_metadata(rt, NAR_META__Nar_Program_execute, &execute);
    nar->set_metadata(rt, NAR_META__Nar_Program_cmd_new, &cmd_new);
    nar->set_metadata(rt, NAR_META__Nar_Program_sub_new, &sub_new);
    nar->set_metadata(rt, NAR_META__Nar_Program_task_new, &task_new);
    nar->set_metadata(rt, NAR_META__Nar_Program_flush, &flush);
    return 0;
}
