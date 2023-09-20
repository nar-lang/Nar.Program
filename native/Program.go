package program

import "github.com/oaklang/core"

definedType Program_Program func() int

func Program_script(fn Program_ScriptFn) Program_Program {
	return func() int {
		return int(fn(core.List_List[core.String_String]{}))
	}
}
