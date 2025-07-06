# speculative non-lisp syntax (early stages).

# simple sequence interface
fun sequence?(xs)
  val xs_type = typeof(xs)

  xs_type == :List or xs_type == :Map or xs_type == :Bin or xs_type == :Str
end

fun count(xs)
  val xs_type = typeof(xs)

  if xs_type == :List
    list_count(xs)
  elif xs_type == :Map
    map_count(xs)
  elif xs_type == :Str
    str_count(xs)
  elif xs_type == :Bin
    bin_count(xs)
  else
    error("not a sequence.")
  end
end

fun apply_sequence_transform(fn, xs, loop)
  # Helper for map, filter, etc. Factors out the repetitive type checking logic.
  # loop should have signature (fn, xs, off, acc, ctor)

  val xs_type = typeof(xs)

  if xs_type == :List
    loop(fn, xs, 0, MutList(), to_list)
  elif xs_type == :Map
    loop(fn, map_entries(xs), 0, MutList(), to_map)
  elif xs_type == :Str
    loop(fn, xs, 0, MutList(), to_str)
  elif xs_type == :Bin
    loop(fn, xs, 0, MutBin(), to_bin)
  else
    error("not a sequence.")
  end
end

fun map(fn, xs)
  fun loop(fn, xs, off, acc, ctor)
    if off == count(xs) # no more
      ctor(acc)
    else
      mut_list_push!(acc, fn(xs[off]))
      loop(fn, xs, off+1, acc, ctor)
    end
  end

  apply_sequence_transform(fn, xs, loop)
end

fun filter(p?, xs)
  fun loop(fn, xs, off, acc, ctor)
    if off == count(xs)
      ctor(acc)
    elif p?(xs[off])
      mut_list_push!(acc, xs[off])
      loop(fn, xs, off+1, acc, ctor)
    else
      loop(fn, xs, off+1, acc, ctor)
    end
  end

  apply_sequence_transform(p?, xs, loop)
end

fun take_while(p?, xs)
  fun loop(p?, xs, off, acc, ctor)
    if off == count(xs)
      ctor(acc)
    elif not p?(xs[off])
      ctor(acc)
    else
      mut_list_push!(acc, xs[off])
      loop(p?, xs, off+1, acc, ctor)
    end
  end

  apply_sequence_transform(p?, xs, loop)
end