#include "_package.h"

nar_t *nar;

nar_int_t init(nar_t *n, nar_runtime_t rt) {
    nar = n;
    nar->register_def_dynamic(rt, "Nar.Program.Cmd", "map", "cmd_map", 2);
    nar->register_def_dynamic(rt, "Nar.Program", "application", "program_application", 1);
    nar->register_def_dynamic(rt, "Nar.Program", "exit", "program_exit", 1);
    nar->register_def_dynamic(rt, "Nar.Program.Sub", "map", "sub_map", 2);
    nar->register_def_dynamic(rt, "Nar.Program.Task", "succeed", "task_succeed", 1);
    nar->register_def_dynamic(rt, "Nar.Program.Task", "fail", "task_fail", 1);
    nar->register_def_dynamic(rt, "Nar.Program.Task", "andThen", "task_andThen", 2);
    nar->register_def_dynamic(rt, "Nar.Program.Task", "onError", "task_onError", 2);
    nar->register_def_dynamic(rt, "Nar.Program.Task", "attempt", "task_attempt", 2);

    nar->set_metadata(rt, NAR_META_NAR_PROGRAM_EXECUTE, &execute);
    nar->set_metadata(rt, NAR_META_NAR_PROGRAM_CMD_NEW, &cmd_new);
    nar->set_metadata(rt, NAR_META_NAR_PROGRAM_SUB_NEW, &sub_new);
    nar->set_metadata(rt, NAR_META_NAR_PROGRAM_FLUSH, &flush);
    return 0;
}
