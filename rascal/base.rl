provide Base
require Core


fun isa?(x, t: Type) do
  instance?(typeof(x), t)
end

fun map(fn: Fun, xs: List) do
  label loop fn = fn, xs = xs, acc = [] do
    if empty? xs do
      reverse acc
    else
      let [first | rest] = xs do
        loop fn, rest, fn(first) | acc
      end
    end
  end
end

fun map(fn: Fun, kvs: Map) do
  label loop fn = fn, kvs = pairs(kvs), acc = {} do
    if empty?(kvs) do
      acc
    else
      let [first | rest] = kvs do
        loop fn, rest, fn(first) | acc
      end
    end
  end
end

mac try(do: try_block, catch: catch_block) do
  fun xform_catch_pattern(pattern) do
    let (etype, msg) = pattern do
      quote (qualify(etype, :exception), msg, _)
    end
  end

  fun xform_catch_clauses(block) do
    syntax_map(xform_catch_pattern, block)
  end

  quote do
    control do
      splice(try_block)
    handle
      splice(xform_catch_clauses(catch_block))
    end
  end
end