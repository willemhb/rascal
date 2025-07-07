provide Base


fun isa?(x, t) do
  # simple type checking utility
  typeof(x) == t
end

fun Set(items) do
  # constructor for set pseudo-type
  if not isa?(items, :List) do
    error("not a list")
  end

  fun loop(xs, off, acc) do
    if off == count(xs) do
      to_map(acc)
    else
      loop(xs, off+1, mut_map_set!(acc, xs[off], true))
    end
  end

  loop(xs, 0, MutMap())
end

