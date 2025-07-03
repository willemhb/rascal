# speculative non-lisp syntax.


fun fib(n: Int): Int do
  "Calculate the `nth` element of the Fibonacci sequence."
  match n do
    when n < 0 -> raise :value-error, "${n} is less than zero."
    case 0     -> 1
    case 1     -> 1
    case n     -> fib(n - 2) + fib(n - 1)
  end
end

fun has?(xs: List{X}, x: X): Bool do
  match xs, x do
    [], _     -> false
    [_|x], x  -> true
    [xs|_], x -> has? xs, x
  end
end

# example type definitions

# example structure type definitions (stored as tuples)
type Pair{X, Y} = (first: X, second: Y)

type Fraction = (numer: Integer, denom: Integer)

# example union type definitions
type Number = Real | Integer | Fraction | Complex

# example enum type definition
type Gender = :Male | :Female | :NonBinary

# example record type definition (stored as maps, may contain other keys)
type Person = { name: String, age: Integer, gender: Gender }

# example algebraic type definition
type SList{X} = Empty() | Cons(head: X, tail: SList{X})

# example macro definitions
mac throw(exc: Symbol) do
  quote
    raise $exc, "exception raised."
  end
end

mac throw(exc: Symbol, msg: String) do
  quote
    raise $exc, $msg
  end
end

mac try(exprs) do
  if 
end

mac yield(xpr) do
  raise :yield, xpr
end

mac generator(body) do
  quote
    
  end
end