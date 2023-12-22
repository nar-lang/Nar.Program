export default function (runtime) {
    const program = {
        opt: {},
        msgs: [],
        subs: [],
        cmds: [],
        model: null,
    }

    let cycleScheduled = false;

    function postMsg(msg) {
        program.msgs.push(msg);
        scheduleCycle();
    }

    /**
     * @param {Readonly<{}>} _toMsg oak function that converts data to a message
     * @param {function(post:function(data))} exec function that executes the command posts the data,
     * which will be converted to a message and dispatched to update
     * @return {*}
     */
    function newCmd(_toMsg, exec) {
        return runtime.native([{
            toMsg: (_data) => runtime.executeFn(_toMsg, [_data]),
            exec
        }]);
    }

    function init() {
        const result = runtime.executeFn(program.opt.init, [runtime.list([])]); //TODO: pass request params
        processUpdateResult(result);
    }

    function flushMessages() {
        for (let i = 0; i < program.msgs.length; i++) {
            const msg = program.msgs[i];
            const result = runtime.executeFn(program.opt.update, [msg, program.model]);
            processUpdateResult(result);
        }
        program.msgs = [];
    }

    function processUpdateResult(result) {
        program.model = result.value[0];
        const cmds = runtime.unwrapShallow(result.value[1]);
        program.cmds.push(...cmds);
    }

    function flushCommands() {
        program.cmds.forEach(cmd => cmd.exec(data => finishCmd(cmd, data)));
        program.cmds = [];
    }

    function viewAndPresent() {
        const view = runtime.executeFn(program.opt.view, [program.model]);
        runtime.executeFn(program.opt.present, [view]);
    }

    function cycle() {
        cycleScheduled = false;
        while (program.msgs.length > 0 || program.cmds.length > 0) {
            flushMessages();
            flushCommands();
        }
        viewAndPresent();
    }

    function scheduleCycle() {
        if (!cycleScheduled) {
            cycleScheduled = true;
            setTimeout(cycle, 0);
        }
    }

    function finishCmd(cmd, data) {
        const msg = cmd.toMsg(data);
        postMsg(msg);
    }

    /**
     * @param {function(success:function(_result), fail:function(_error))} exec
     */
    function newTask(exec) {
        return runtime.native(exec);
    }

    runtime.register("Oak.Program.Cmd", {
        "batch": (_ls) => {
            const cmds = runtime.unwrapShallow(_ls);
            return runtime.native(cmds.flat());
        },
        "map": (_fn, _cmd) => {
            const cmds = runtime.unwrap(_cmd);
            const mapped = cmds.map(cmd => ({
                exec: cmd.exec,
                toMsg: (_data) => {
                    _data = cmd.toMsg(_data);
                    runtime.executeFn(_fn, [_data]);
                }
            }));
            return runtime.native(mapped);
        }
    });
    runtime.register("Oak.Program.Sub", {
        "batch": (_ls) => {
            return runtime.native(runtime.unwrapShallow(_ls));
        },
        "map": (_fn, _ls) => {
            return runtime.native(
                _ls.value.map(x => Object.freeze({
                    sub: x.cmd,
                    maps: [...x.maps, _fn]
                }))
            );
        }
    });
    runtime.register("Oak.Program.Msg", {
        "batch": (_ls) => {
            return runtime.native(runtime.unwrapShallow(_ls));
        },
        "map": (_fn, _ls) => {
            return runtime.native(
                _ls.value.map(x => Object.freeze({
                    cmd: x.cmd,
                    maps: [...x.maps, _fn]
                }))
            );
        }
    });
    runtime.register("Oak.Program", {
        "application": (_opt) => {
            return runtime.native(runtime.unwrapShallow(_opt));
        },
        "execute": (_opt) => {
            program.opt = _opt.value;
            init();
            scheduleCycle();
            return runtime.int(0); //TODO: return exit code
        }
    })
    runtime.register("Oak.Program.TextPresenter", {
        "println": (_text) => {
            console.log(runtime.unwrap(_text));
            return runtime.unit();
        }
    });
    runtime.register("Oak.Program.Task", {
        "succeed": (_result) => newTask((success, _) => success(_result)),
        "fail": (_error) => newTask((_, fail) => fail(_error)),
        "andThen": (_fn, _task) => {
            const task = runtime.unwrap(_task);
            return newTask((success, fail) => {
                task.exec(
                    (_result) => {
                        const _nextTask = runtime.executeFn(_fn, [_result]);
                        const nextTask = runtime.unwrap(_nextTask);
                        nextTask.exec(success, fail);
                    },
                    fail
                );
            });
        },
        "onError": (_fn, _task) => {
            const task = runtime.unwrap(_task);
            return newTask((success, fail) => {
                task.exec(
                    success,
                    (_error) => {
                        const _nextTask = runtime.executeFn(_fn, [_error]);
                        const nextTask = runtime.unwrap(_nextTask);
                        nextTask.exec(success, fail);
                    }
                );
            });
        },
        "attempt": (_toMsg, _task) => {
            const task = runtime.unwrap(_task);
            return newCmd(_toMsg, (post) => {
                task(
                    (_result) => post(runtime.optionShallow("Oak.Core.Result.Result", "Ok", [_result])),
                    (_error) => post(runtime.optionShallow("Oak.Core.Result.Result", "Err", [_error]))
                )
            });
        }
    });

    runtime.scope("Oak.Program", {newCmd, newTask})
}
