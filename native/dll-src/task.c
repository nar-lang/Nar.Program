#include "_package.h"

nar_object_t task_succeed(nar_runtime_t rt, nar_object_t result) {}

nar_object_t task_fail(nar_runtime_t rt, nar_object_t error) {}

nar_object_t task_andThen(nar_runtime_t rt, nar_object_t f, nar_object_t task) {}

nar_object_t task_onError(nar_runtime_t rt, nar_object_t f, nar_object_t task) {}

nar_object_t task_attempt(nar_runtime_t rt, nar_object_t resultToMessage, nar_object_t task) {}
