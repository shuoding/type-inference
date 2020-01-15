# type-inference
This repository contains a very simple example of static type inference and static type check.

## Examples
```
...> (let x = 1 in x)
x :: INT
...> (if x then 0 else 1)
x :: BOOL
...> (let x = (if true then y else z) in x)
x :: GENERICS-5
y :: GENERICS-5
z :: GENERICS-5
...> (let x = y in (let z = w in 0))
w :: GENERICS-5
x :: GENERICS-2
y :: GENERICS-2
z :: GENERICS-5
...> (- 1 0)
...> (- 1 x)
x :: INT
...> (if true then false else 0)
Type Error: cannot unify BOOL and INT
```

## Grammar (LL1)
```
<expr> := <variable> # any non-empty alphabetic sequences except for boolean literals and keywords
                     # duplicate variable names are not supported
        | <integer> # 0 | 1 | -1 | ...
                    # - 1 is invalid. The digits must immediately follow the negative sign.
        | <boolean> # true | false
        | ( - <expr1> <expr2> )
        | ( * <expr1> <expr2> )
        | ( / <expr1> <expr2> )
        | ( < <expr1> <expr2> )
        | ( if <expr1> then <expr2> else <expr3> )
        | ( let <variable> = <expr1> in <expr2> )
```

## Type Constraints ([] represents the whole expression)
```
<variable>                               :
<integer>                                : [] = INT
<boolean>                                : [] = BOOL
( - <expr1> <expr2> )                    : [] = INT, [<expr1>] = [<expr2>] = INT
( * <expr1> <expr2> )                    : [] = INT, [<expr1>] = [<expr2>] = INT
( / <expr1> <expr2> )                    : [] = INT, [<expr1>] = [<expr2>] = INT
( < <expr1> <expr2> )                    : [] = BOOL, [<expr1>] = [<expr2>] = INT
( if <expr1> then <expr2> else <expr3> ) : [] = [<expr2>], [<expr1>] = BOOL, [<expr2>] = [<expr3>]
( let <variable> = <expr1> in <expr2> )  : [] = [<expr2>], [<variable>] = [<expr1>]
```
