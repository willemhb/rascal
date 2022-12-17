;; start nolisp.rl

;; type definitions

type nul = ()
type cons = { car, cdr }
type list = nul | cons

fun list() = nul
fun list(x, rest..) = cons(x, list(..rest))

type empty = ()
type leaf = { key, val }
type node = { left, right }
type tree = empty | leaf | node

type regexp = { pattern, flags, compiled }

fun regexp(pattern) = regexp(pattern, 0)
fun regexp(pattern, flags) = regexp(pattern, flags, comp_re(pattern, flags))

fun map (fn, xs:list)
    cond
        xs == ()  => ()
        otherwise => cons(fn(xs.car), map(fn, xs.cdr))
    end
end

fun filter(fn?, xs:list)
    cond
        xs == ()    => ()
        fn?(xs.car) => cons(xs.car, filter(fn?, xs.cdr))
        otherwise   => filter(fn?, xs.cdr)
    end
end

fun length(xs:list)
    case xs
        ()          => 0
        cons(_, cd) => 1 + length(cd)
    end
end

export list, tree, regexp, map, filter, length
