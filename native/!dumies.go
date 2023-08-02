//go:build oak_lib_dev

package program

import "github.com/oaklang/core"

type Program_ScriptFn func(list core.List_List[core.String_String]) core.Basics_Int
