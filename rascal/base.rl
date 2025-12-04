provide Base
require Core


fun isa?(x, t: Type) do
  instance?(typeof(x), t)
end

fun map(fn: Fun, xs: List) do
  label loop fn = fn, xs = vals!(xs), acc = transient!([]) do
    if empty? xs do
      persistent(acc)
    else
      ref (first, rest) = next!(vals)
      loop fn, rest, fn(first) | acc
    end
  end
end

fun map(fn: Fun, kvs: Map) do
  label loop fn = fn, kvs = pairs!(kvs), acc = transient!({}) do
    if empty?(kvs) do
      persistent(acc)
    else
      ref (kv, rest) = next!(kvs)
      loop fn, rest, fn(kv) | acc
    end
  end
end

fun elem?(x, xs: List) do
  
end
