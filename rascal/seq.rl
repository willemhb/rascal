provide Seq

# simple sequence interface
fun seq?(xs)
  val xs_type = typeof(xs)

  xs_type == :List or xs_type == :Map or xs_type == :Bin or xs_type == :Str
end

fun seq_type(xs)
  # Helper for discriminating sequences from non-sequences.
  val xs_type = typeof(xs)

  if xs_type == :List or xs_type == :Bin or xs_type == :Str
    :ordered
  else if xs_type == :Map
    :unordered
  else
    :atom
  end
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

fun seq_constructor(xs)
  # helper for sequence transformations
  # either return a function 
  val xs_type = typeof(xs)

  if xs_type == :List
    to_list
  elif xs_type == :Map
    to_map
  elif xs_type == :Str
    to_str
  elif xs_type == :Bin
    to_bin
  else
    error("not a sequence.")
  end
end

fun linearize_seq(xs)
  val instance = seq_type(xs)

  if instance == :ordered
    xs
  elif instance == :unordered
    map_items(xs)
  else
    error("not a sequence.")
  end
end

fun transform_sequence(fn, xs, loop)
  # Helper for map, filter, etc. Factors out the repetitive type checking logic.
  # loop should have signature (fn, xs, off, acc, ctor)

  val xs_type = typeof(xs)

  if xs_type == :List
    loop(fn, xs, 0, MutList(), to_list)
  elif xs_type == :Map
    loop(fn, map_items(xs), 0, MutList(), to_map)
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

  transform_sequence(fn, xs, loop)
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

  transform_sequence(p?, xs, loop)
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

  transform_sequence(p?, xs, loop)
end

fun reduce(fn, xs, init)
  val items = linearize_seq(xs)

  fun loop(fn, xs, off, acc)
    if off == count(xs)
      acc
    else
      loop(fn, xs, off+1, fn(acc, xs[off]))
    end
  end

  loop(fn, items, 0, init)
end

fun drop_while(p?, xs)
  fun loop(p?, xs, off, slicer)
    
  end
end
