export default function (runtime) {
    runtime.register("Oak.Program", {
        "script": function (fn) {
            runtime.executeFn(fn, [runtime.list([])]);
            return {};
        }
    });
}
