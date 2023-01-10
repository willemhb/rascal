;; start nolisp.rl

;; type definitions
type list{X} = | nul  {}
     	       | cons { head: X, tail: list }

type tree{X} = | empty {}
	       | node  { key: X, left: tree, right: tree }

fun list() = ()
fun list(x, rest..) = cons(x, list(..rest))

fun tree() = empty()
fun tree(k, r..) = insert(k, tree(..r))

fun insert(k, _: empty) = node{k, empty(), empty() }
fun insert(k, n: node{k, l, r}) =
    case ord(k, n.k)
    	 :lt => node(n.k, insert(k, l), r),
	 :rt => node(n.k, l, insert(k, r)),
	 :eq => n

fun len(nul{}) = 0
fun len(cons{_, t}) = 1 + len(t)

fun assc(_, nul{}) = nul()
fun assc(x, cons{kv: cons{x, _}, _}) = kv
fun assc(x, cons{_, rest}) = assc(x, rest)

fun assc(k, xs: vec) =
    fun helper(k, l, xs) =
    	if l == len(xs)
	   nul()
	else
	   let kv: {k0, _} = xs[l]
	       if k == k0
	       	  kv
	       else
		  helper(k, l+1, xs)
    helper(k, 0, xs)

provide list, tree, regexp, map, filter, length
