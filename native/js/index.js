export default function (runtime) {
    /**
     * @param {function(success:function(_result), fail:function(_error))} exec
     */
    function newTask(exec) {
        return runtime.native(exec);
    }

    /**
     * @param {Readonly<{}>} _toMsg  function that converts data to a message
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

    /**
     * @param {string} id
     * @param {[Readonly<{}>]} args
     * @param {Readonly<{}>} _toMsg  function that converts data to a message
     * @param {function(args: [Readonly<{}>], post:function(data)):function} subscribe returns unsubscribe
     */
    function newSub(id, args, _toMsg, subscribe) {
        return runtime.native([{
            id,
            args,
            toMsg: (_data) => runtime.executeFn(_toMsg, [_data]),
            sub: subscribe
        }]);
    }

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

    function subscribe() {
        const _newSubs = runtime.executeFn(program.opt.subscribe, [program.model]);
        const newSubs = runtime.unwrapShallow(_newSubs);
        const nextSubs = [];
        const cmpList = runtime.scope("Nar.Base").cmpList;
        for (let i = 0; i < newSubs.length; i++) {
            let newSub = newSubs[i];
            let survived = false;
            for (let j = 0; j < program.subs.length; j++) {
                let sub = program.subs[j];
                if (sub.id === newSub.id && cmpList(sub.args, newSub.args) === 0) {
                    nextSubs.push(sub);
                    program.subs[j] = null;
                    survived = true;
                    break;
                }
            }
            if (!survived) {
                nextSubs.push(newSub);
                newSub.unsub = newSub.sub(newSub.args, data => {
                    postMsg(newSub.toMsg(data));
                });
            }
        }
        for (let j = 0; j < program.subs.length; j++) {
            let sub = program.subs[j];
            if (sub !== null) {
                sub.unsub();
            }
        }
        program.subs = nextSubs;
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
        subscribe();
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

    runtime.scope("Nar.Program", {newCmd, newTask, newSub})

    runtime.register("Nar.Program.Cmd", {
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
    runtime.register("Nar.Program.Sub", {
        "batch": (_ls) => {
            const subs = runtime.unwrapShallow(_ls);
            return runtime.native(subs.flat());
        },
        "map": (_fn, _sub) => {
            const subs = runtime.unwrap(_sub);
            const mapped = subs.map(sub => ({
                unsub: sub.unsub,
                toMsg: (_data) => {
                    _data = sub.toMsg(_data);
                    runtime.executeFn(_fn, [_data]);
                }
            }));
            return runtime.native(mapped);
        }
    });
    runtime.register("Nar.Program", {
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
    runtime.register("Nar.Program.TextPresenter", {
        "println": (_text) => {
            console.log(runtime.unwrap(_text));
            return runtime.unit();
        }
    });
    runtime.register("Nar.Program.Task", {
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
                    (_result) => post(runtime.optionShallow("Nar.Base.Result.Result", "Ok", [_result])),
                    (_error) => post(runtime.optionShallow("Nar.Base.Result.Result", "Err", [_error]))
                )
            });
        }
    });
}
