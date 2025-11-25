provide Base

fun isa?(x, t) do
  # simple type checking utility
  typeof(x) == t
end

fun Set(items) do
  # constructor for set pseudo-type
  if not isa?(items, :List) do
    throw "not a list"
  end

  label loop xs = xs, off = 0, acc = MutMap() do
    if off == count(xs) do
      to_map(acc)
    else
      loop xs, off+1, mut_map_set!(acc, xs[off], true)
    end
  end
end

